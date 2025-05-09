package main

import (
	"encoding/json"
	"io"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"unsafe"
	"syscall"
	"sync"
	"bufio"
	"exec"
	"fmt"
	"time"
)

// Stockage des logs en mémoire
var (
	logs   []string
	logsMu sync.Mutex
)

var (
	globalDriver string
	globalDevice string
)

// Chemin par défaut du mapping (on pourra l'ajuster si besoin)
var defaultMappingPath string

// getMappingHandler lit et renvoie le contenu de mapping.json
func getMappingHandler(w http.ResponseWriter, r *http.Request) {
	data, err := os.ReadFile(defaultMappingPath)
	if err != nil {
		http.Error(w, "Erreur lors de la lecture du mapping", http.StatusInternalServerError)
		return
	}
	w.Header().Set("Content-Type", "application/json")
	w.Write(data)
}

// mergeMaps effectue une fusion récursive de deux maps.
// Pour toute clé du patch, si la valeur est un objet (map), on tente de fusionner
// avec la valeur existante dans original, sinon on écrase.
func mergeMaps(original, patch map[string]interface{}) map[string]interface{} {
	for key, patchValue := range patch {
		if patchMap, ok := patchValue.(map[string]interface{}); ok {
			if originalValue, exists := original[key]; exists {
				if originalMap, ok2 := originalValue.(map[string]interface{}); ok2 {
					original[key] = mergeMaps(originalMap, patchMap)
					continue
				}
			}
		}
		original[key] = patchValue
	}
	return original
}

// mergeDevice fusionne les données d'un périphérique (device).
// Pour les champs simples, on écrase, pour "axes" et "buttons" on réalise un merge ciblé.
func mergeDevice(orig, patch map[string]interface{}) map[string]interface{} {
	// Mise à jour des champs simples (sauf axes et buttons)
	for key, val := range patch {
		if key == "axes" || key == "buttons" {
			continue
		}
		orig[key] = val
	}

	// Fusion des axes : pour chaque axe du patch, on cherche celui de même code dans orig.
	if patchAxes, ok := patch["axes"].([]interface{}); ok {
		if origAxesRaw, exists := orig["axes"]; exists {
			if origAxes, ok2 := origAxesRaw.([]interface{}); ok2 {
				for _, axisPatchRaw := range patchAxes {
					if axisPatch, ok := axisPatchRaw.(map[string]interface{}); ok {
						if code, exists := axisPatch["code"]; exists {
							found := false
							for i, axisOrigRaw := range origAxes {
								if axisOrig, ok := axisOrigRaw.(map[string]interface{}); ok {
									if origCode, exists2 := axisOrig["code"]; exists2 && origCode == code {
										origAxes[i] = mergeMaps(axisOrig, axisPatch)
										found = true
										break
									}
								}
							}
							// Optionnel : si aucun axe ne correspond, on peut l'ajouter
							if !found {
								origAxes = append(origAxes, axisPatch)
							}
						}
					}
				}
				orig["axes"] = origAxes
			}
		} else {
			orig["axes"] = patchAxes
		}
	}

	// Fusion des boutons : patch["buttons"] est un objet dont chaque clé correspond à un bouton.
	if patchButtons, ok := patch["buttons"].(map[string]interface{}); ok {
		if origButtonsRaw, exists := orig["buttons"]; exists {
			if origButtons, ok2 := origButtonsRaw.(map[string]interface{}); ok2 {
				for btnKey, btnPatchVal := range patchButtons {
					if btnPatch, ok := btnPatchVal.(map[string]interface{}); ok {
						if origButtonRaw, exists2 := origButtons[btnKey]; exists2 {
							if origButton, ok3 := origButtonRaw.(map[string]interface{}); ok3 {
								origButtons[btnKey] = mergeMaps(origButton, btnPatch)
							} else {
								origButtons[btnKey] = btnPatch
							}
						} else {
							origButtons[btnKey] = btnPatch
						}
					}
				}
				orig["buttons"] = origButtons
			}
		} else {
			orig["buttons"] = patchButtons
		}
	}

	return orig
}

// mergeMapping fusionne le mapping global en mettant à jour les clés globales
// et en fusionnant l'array "devices" en identifiant chaque périphérique par son "path".
func mergeMapping(original, patch map[string]interface{}) map[string]interface{} {
	// Mise à jour des clés globales (hors "devices")
	for key, val := range patch {
		if key == "devices" {
			continue
		}
		original[key] = val
	}

	// Fusion de l'array devices
	if patchDevices, ok := patch["devices"].([]interface{}); ok {
		if origDevicesRaw, exists := original["devices"]; exists {
			if origDevices, ok2 := origDevicesRaw.([]interface{}); ok2 {
				for _, devPatchRaw := range patchDevices {
					if devPatch, ok := devPatchRaw.(map[string]interface{}); ok {
						// Utilise le champ "path" comme identifiant unique
						if patchPath, ok := devPatch["path"].(string); ok {
							found := false
							for i, devOrigRaw := range origDevices {
								if devOrig, ok := devOrigRaw.(map[string]interface{}); ok {
									if origPath, ok2 := devOrig["path"].(string); ok2 && origPath == patchPath {
										origDevices[i] = mergeDevice(devOrig, devPatch)
										found = true
										break
									}
								}
							}
							// Optionnel : ajouter le périphérique s'il n'existe pas déjà
							if !found {
								origDevices = append(origDevices, devPatch)
							}
						}
					}
				}
				original["devices"] = origDevices
			}
		} else {
			original["devices"] = patchDevices
		}
	}

	return original
}

func start_c(){
	cmd := exec.Command("raw_joystick", "fe980000.usb", "fe980000.usb")
	cmd.Stdout = os.Stdout
	cmd.Stderr = os.Stderr

	if err := cmd.Start(); err != nil {
		fmt.Printf("Erreur lors du démarrage de l'exécutable: %v\n", err)
		os.Exit(1)
	}

	fmt.Printf("Exécutable lancé avec PID %d\n", cmd.Process.Pid)
}

func ctrl_c(cmd *exec.Cmd, timeout time.Duration) (*exec.Cmd, error) {
    // Envoi du signal SIGINT (équivalent à un Ctrl-C).
    if err := cmd.Process.Signal(syscall.SIGINT); err != nil {
        return nil, fmt.Errorf("envoi du signal SIGINT: %v", err)
    }
    fmt.Println("Signal SIGINT envoyé (équivalent Ctrl-C)")

    done := make(chan error, 1)
    go func() {
        done <- cmd.Wait()
    }()

    select {
    case err := <-done:
        if err != nil {
            return nil, fmt.Errorf("attente de la fin du processus: %v", err)
        }
        fmt.Println("Processus terminé.")
    case <-time.After(timeout):
        // En cas de timeout, on tue le processus brutalement.
        if err := cmd.Process.Kill(); err != nil {
            return nil, fmt.Errorf("échec du kill après timeout: %v", err)
        }
        fmt.Println("Processus tué (timeout).")
    }

    // Redémarrage de l'exécutable.
    newCmd, err := start_c()
    if err != nil {
        return nil, fmt.Errorf("redémarrage de l'exécutable: %v", err)
    }

    return newCmd, nil
}

// postMappingHandler reçoit le JSON de modifications, effectue la fusion avec mapping.json,
// sauvegarde le résultat, recharge le mapping côté C, puis redémarre ep0_loop.
func postMappingHandler(w http.ResponseWriter, r *http.Request) {
	defer r.Body.Close()
	data, err := io.ReadAll(r.Body)
	if err != nil {
		http.Error(w, "Erreur lors de la lecture du corps de requête", http.StatusBadRequest)
		return
	}
	// Décodage du patch JSON reçu
	var patch map[string]interface{}
	if err := json.Unmarshal(data, &patch); err != nil {
		http.Error(w, "Erreur lors du décodage du JSON", http.StatusBadRequest)
		return
	}

	// Lecture du fichier mapping.json existant
	originalData, err := os.ReadFile(defaultMappingPath)
	var original map[string]interface{}
	if err == nil {
		if err := json.Unmarshal(originalData, &original); err != nil {
			original = make(map[string]interface{})
		}
	} else {
		original = make(map[string]interface{})
	}

	// Fusionner le mapping existant avec le patch reçu (fonction mergeMapping)
	mergedMapping := mergeMapping(original, patch)

	// Encodage avec indentation pour sauvegarde
	out, err := json.MarshalIndent(mergedMapping, "", "  ")
	if err != nil {
		http.Error(w, "Erreur lors de l'encodage du JSON", http.StatusInternalServerError)
		return
	}
	if newCmd, err := ctrl_c(cmd, timeout); err != nil {
		fmt.Printf("Erreur lors du ctrl-c: %v\n", err)
		return
	} else {
		cmd = newCmd
	}

	// Sauvegarde dans le fichier mapping.json
	if err := os.WriteFile(defaultMappingPath, out, 0644); err != nil {
		http.Error(w, "Erreur lors de la sauvegarde du mapping", http.StatusInternalServerError)
		return
	}

	w.Write([]byte("Mapping sauvegardé, rechargé et ep0_loop redémarré avec succès."))
}


// mappingHandler redirige selon GET ou POST
func mappingHandler(w http.ResponseWriter, r *http.Request) {
	switch r.Method {
	case http.MethodGet:
		getMappingHandler(w, r)
	case http.MethodPost:
		postMappingHandler(w, r)
	default:
		http.Error(w, "Méthode non autorisée", http.StatusMethodNotAllowed)
	}
}

func logsAPIHandler(w http.ResponseWriter, r *http.Request) {
	logsMu.Lock()
	// Copie des logs pour éviter de bloquer la slice pendant l'encodage
	currentLogs := make([]string, len(logs))
	copy(currentLogs, logs)
	logsMu.Unlock()

	response := struct {
		Logs []string `json:"logs"`
	}{
		Logs: currentLogs,
	}

	w.Header().Set("Content-Type", "application/json")
	if err := json.NewEncoder(w).Encode(response); err != nil {
		http.Error(w, "Erreur lors de l'encodage JSON", http.StatusInternalServerError)
	}
}


func main() {
	// Détermine le chemin absolu de l'exécutable
	exePath, err := os.Executable()
	if err != nil {
		log.Fatalf("readlink error: %v", err)
	}
	dir := filepath.Dir(exePath)
	// On suppose que mapping.json se trouve à côté de l'exécutable
	defaultMappingPath = filepath.Join(dir, "mapping.json")
	log.Printf("Chemin du mapping: %s\n", defaultMappingPath)

	cmd, err := start_c()
	if err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	// Configuration du serveur HTTP (fichiers statiques, API /mapping, etc.)
	fs := http.FileServer(http.Dir("/home/mokin/raw_joystick/app/public"))
	http.Handle("/", fs)
	http.HandleFunc("/mapping", mappingHandler)
	http.HandleFunc("/api/logs", logsAPIHandler)

	// Si le fichier mapping.json n'existe pas, le créer
	if _, err := os.Stat(defaultMappingPath); os.IsNotExist(err) {
		os.WriteFile(defaultMappingPath, []byte("{}"), 0644)
	}

	// Redirection de stdout pour capturer les logs
	logReader, err := redirectStdout()
	if err != nil {
		log.Fatalf("Erreur lors de la redirection de stdout : %v", err)
	}
	go captureLogs(logReader)

	log.Println("Serveur démarré sur http://localhost:3000")
	if err := http.ListenAndServe(":3000", nil); err != nil {
		log.Fatal(err)
	}
}
