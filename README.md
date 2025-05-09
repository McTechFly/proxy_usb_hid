# Raw Joystick Dashboard

## Description

**Raw Joystick Dashboard** est une interface utilisateur basée sur le **Soft UI Dashboard 3** de Creative Tim. Ce projet fournit une interface web pour configurer et mapper les axes et boutons d'un joystick USB. Il utilise des graphiques interactifs et des composants modernes pour une expérience utilisateur fluide.

## Fonctionnalités

- **Mapping des axes et boutons** : Configurez les axes et boutons du joystick via une interface intuitive.
- **Graphiques interactifs** : Visualisez les données avec des graphiques dynamiques grâce à Chart.js.
- **Personnalisation de l'interface** : Modifiez les couleurs de la barre latérale et le type de navigation.
- **Support multi-plateforme** : Compatible avec les navigateurs modernes et les plateformes Windows.

## Structure du projet

LICENSE Makefile mapping.json README.md app/ 5564523-200.png main.go public/ index.html script.js style.css assets/ css/ fonts/ img/ js/ scss/ include/ ep0.h input_mapping.h usb_debug.h usb_descriptors.h usb_hid.h usb_raw.h src/ ep0.c globals.c input_mapping.c main.c usb_debug.c usb_descriptors.c usb_hid.c usb_raw.c


## Prérequis

- **Node.js** : Pour gérer les dépendances front-end.
- **Go** : Pour exécuter le backend.
- **Navigateur moderne** : Pour accéder à l'interface utilisateur.

## Installation

1. Clonez le dépôt :
   ```bash
   git clone https://github.com/votre-utilisateur/raw-joystick-dashboard.git
   cd raw-joystick-dashboard
2. Installez les dépendances front-end :
   ```bash
   npm install
   ```
3. Compilez le projet Go :
   ```bash
   go build -o raw-joystick-dashboard main.go
   ``` 
Contribution

Les contributions sont les bienvenues ! Veuillez suivre ces étapes :
1. Fork le projet.
2. Créez une nouvelle branche (`git checkout -b feature/YourFeature`).
3. Apportez vos modifications et validez (`git commit -m 'Add some feature'`).
4. Poussez vers la branche (`git push origin feature/YourFeature`).
5. Ouvrez une Pull Request.
## Auteurs
- **Votre Nom** - *Développeur principal* - [Votre Profil GitHub]

Licence

Ce projet est sous licence MIT. Consultez le fichier LICENSE pour plus d'informations.