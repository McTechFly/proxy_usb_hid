let globalMapping = {};
let selectedDeviceIndex = 0;

async function loadMapping() {
  try {
    const response = await fetch('/mapping');
    if (!response.ok) {
      throw new Error('Erreur de chargement du mapping');
    }
    globalMapping = await response.json();
  } catch (error) {
    console.error("Erreur lors du chargement du mapping, utilisation de valeurs par défaut:", error);
    globalMapping = { devices: [] }; // Valeur par défaut si l'API échoue
  }
}

/**
 * Renvoie le plus long préfixe commun (mot par mot) entre 2 chaînes.
 */
function longestCommonPrefixWords(str1, str2) {
  const words1 = str1.split(' ');
  const words2 = str2.split(' ');
  const common = [];
  for (let i = 0; i < Math.min(words1.length, words2.length); i++) {
    if (words1[i].toLowerCase() === words2[i].toLowerCase()) {
      common.push(words1[i]);
    } else {
      break;
    }
  }
  return common.join(' ');
}

/**
 * Normalise un nom en retirant les espaces superflus.
 */
function normalizeName(name) {
  return name.trim().replace(/\s+/g, ' ');
}

/**
 * Pour un périphérique donné, si d'autres périphériques partagent un préfixe commun
 * (mais que les noms ne sont pas strictement identiques), retourne le nom sans ce préfixe commun.
 * Sinon, le nom original est conservé.
 */
function computeTrimmedName(name, allNames) {
  const normalizedName = normalizeName(name);
  // Transformation des noms de tous les périphériques en version normalisée.
  const normalizedNames = allNames.map(n => normalizeName(n));
  // On sélectionne les autres noms qui partagent le même premier mot.
  const similarNames = normalizedNames.filter(n => n !== normalizedName && n.split(' ')[0].toLowerCase() === normalizedName.split(' ')[0].toLowerCase());
  if (similarNames.length === 0) return normalizedName;
  
  // Regroupe le nom courant et les noms similaires.
  const group = [normalizedName, ...similarNames];
  // Calcule le préfixe commun à tous les noms du groupe.
  const lcp = group.reduce((prefix, current) => longestCommonPrefixWords(prefix, current));
  
  // Si le préfixe commun correspond à l'intégralité du nom (cas de périphériques identiques), on conserve le nom original.
  if (lcp.trim().toLowerCase() === normalizedName.trim().toLowerCase()) return normalizedName;
  
  // Sinon, on retire le préfixe commun et on retourne le reste.
  if (lcp && normalizedName.startsWith(lcp)) {
    const trimmed = normalizedName.substring(lcp.length).trim();
    return trimmed || normalizedName;
  }
  return normalizedName;
}

function renderDeviceTabs() {
  const tabsContainer = document.getElementById('device-tabs');
  tabsContainer.innerHTML = '';

  if (!globalMapping.devices || globalMapping.devices.length === 0) {
    tabsContainer.innerHTML = '<p>Aucun périphérique disponible</p>';
    return;
  }
  
  // Récupération de tous les noms pour le traitement.
  const allNames = globalMapping.devices.map(device => device.name || "");

  globalMapping.devices.forEach((device, index) => {
    const li = document.createElement('li');
    li.className = 'nav-item';

    const a = document.createElement('a');
    a.href = "#"; // Lien factice
    a.className = 'nav-link' + (index === selectedDeviceIndex ? ' active' : '');

    a.addEventListener('click', function(e) {
      e.preventDefault();
      selectedDeviceIndex = index;
      renderDeviceTabs(); // Actualise la classe active
      renderSidebar();    // Met à jour l'interface de mapping pour le périphérique sélectionné
    });

    // Création de l'icône (SVG)
    const iconDiv = document.createElement('div');
    iconDiv.className =
      'icon icon-shape icon-sm shadow border-radius-md bg-white text-center me-2 d-flex align-items-center justify-content-center';
    iconDiv.innerHTML = `
      <svg width="12px" height="12px" viewBox="0 0 40 40" version="1.1" xmlns="http://www.w3.org/2000/svg"
           xmlns:xlink="http://www.w3.org/1999/xlink">
        <title>settings</title>
        <g stroke="none" stroke-width="1" fill="none" fill-rule="evenodd">
          <g transform="translate(-2020.000000, -442.000000)" fill="#FFFFFF" fill-rule="nonzero">
            <g transform="translate(1716.000000, 291.000000)">
              <g transform="translate(304.000000, 151.000000)">
                <polygon class="color-background opacity-6"
                  points="18.0883333 15.7316667 11.1783333 8.82166667 13.3333333 6.66666667 6.66666667 0 0 6.66666667 6.66666667 13.3333333 8.82166667 11.1783333 15.315 17.6716667">
                </polygon>
                <path class="color-background opacity-6"
                  d="M31.5666667,23.2333333 C31.0516667,23.2933333 30.53,23.3333333 30,23.3333333 C29.4916667,23.3333333 28.9866667,23.3033333 28.48,23.245 L22.4116667,30.7433333 L29.9416667,38.2733333 C32.2433333,40.575 35.9733333,40.575 38.275,38.2733333 L38.275,38.2733333 C40.5766667,35.9716667 40.5766667,32.2416667 38.275,29.94 L31.5666667,23.2333333 Z">
                </path>
                <path class="color-background"
                  d="M33.785,11.285 L28.715,6.215 L34.0616667,0.868333333 C32.82,0.315 31.4483333,0 30,0 C24.4766667,0 20,4.47666667 20,10 C20,10.99 20.1483333,11.9433333 20.4166667,12.8466667 L2.435,27.3966667 C0.95,28.7083333 0.0633333333,30.595 0.00333333333,32.5733333 C-0.0583333333,34.5533333 0.71,36.4916667 2.11,37.89 C3.47,39.2516667 5.27833333,40 7.20166667,40 C9.26666667,40 11.2366667,39.1133333 12.6033333,37.565 L27.1533333,19.5833333 C28.0566667,19.8516667 29.01,20 30,20 C35.5233333,20 40,15.5233333 40,10 C40,8.55166667 39.685,7.18 39.1316667,5.93666667 L33.785,11.285 Z">
                </path>
              </g>
            </g>
          </g>
        </g>
      </svg>
    `;
    a.appendChild(iconDiv);

    // Calcul du nom affiché en retirant le préfixe commun si nécessaire
    const span = document.createElement('span');
    span.className = 'nav-link-text ms-1';
    span.textContent = computeTrimmedName(device.name, allNames);
    a.appendChild(span);

    li.appendChild(a);
    tabsContainer.appendChild(li);
  });
}

function renderSidebar() {
  // Met à jour l'interface de mapping pour le périphérique sélectionné
  generateMappingUI();
  generateButtonsUI();
}

function generateMappingUI() {
  const container = document.getElementById('mappingContainer');
  container.innerHTML = '';

  const device = globalMapping.devices[selectedDeviceIndex];
  if (!device) {
    container.innerHTML = '<p>Périphérique non trouvé</p>';
    return;
  }

  let currentRow = null;
  device.axes.forEach((axis, axisIndex) => {
    if (axisIndex % 6 === 0) {
      currentRow = document.createElement('div');
      currentRow.className = 'row mt-4';
      container.appendChild(currentRow);
    }

    // Bloc de bouton collapsible pour l'axe
    const colDiv = document.createElement('div');
    colDiv.className = 'col-lg-2 col-md-8 col-12 mt-md-0';
    colDiv.setAttribute('data-bs-toggle', 'collapse');
    colDiv.setAttribute('href', `#axeExample_${selectedDeviceIndex}_${axisIndex}`);
    colDiv.setAttribute('role', 'button');
    colDiv.setAttribute('aria-expanded', 'false');
    colDiv.setAttribute('aria-controls', `collapseExample_${selectedDeviceIndex}_${axisIndex}`);

    // Carte principale
    const card = document.createElement('div');
    card.className = 'card';

    const spanMask = document.createElement('span');
    spanMask.className = 'mask bg-dark opacity-10 border-radius-lg';
    card.appendChild(spanMask);

    const cardBody = document.createElement('div');
    cardBody.className = 'card-body p-3 position-relative';
    card.appendChild(cardBody);

    const innerRow = document.createElement('div');
    innerRow.className = 'row';
    cardBody.appendChild(innerRow);

    const col8 = document.createElement('div');
    col8.className = 'col-8 text-start';
    innerRow.appendChild(col8);

    const h5 = document.createElement('h5');
    h5.className = 'text-white font-weight-bolder mb-0 mt-3';
    h5.textContent = axis.code;
    col8.appendChild(h5);

    colDiv.appendChild(card);
    currentRow.appendChild(colDiv);

    // Contenu collapsible pour l'axe
    const collapseDiv = document.createElement('div');
    collapseDiv.className = 'collapse';
    collapseDiv.id = `axeExample_${selectedDeviceIndex}_${axisIndex}`;

    const collapseCard = document.createElement('div');
    collapseCard.className = 'card card-body';
    collapseDiv.appendChild(collapseCard);

    const form = document.createElement('form');
    collapseCard.appendChild(form);

    const formRow = document.createElement('div');
    formRow.className = 'row';
    form.appendChild(formRow);

    const colMd12 = document.createElement('div');
    colMd12.className = 'col-md-12';
    formRow.appendChild(colMd12);

    const formGroup = document.createElement('div');
    formGroup.className = 'form-group';
    colMd12.appendChild(formGroup);

    // Titre du collapse indiquant le code de l'axe
    const h5Form = document.createElement('h5');
    h5Form.textContent = `Axe : ${axis.code}`;
    formGroup.appendChild(h5Form);

    // Champ : Dead Zone
    const labelDeadZone = document.createElement('label');
    labelDeadZone.textContent = 'Dead Zone:';
    formGroup.appendChild(labelDeadZone);

    const inputDeadZone = document.createElement('input');
    inputDeadZone.className = 'form-control';
    inputDeadZone.type = 'text';
    inputDeadZone.value = axis.dead_zone || 0;
    inputDeadZone.id = `deadZone_${selectedDeviceIndex}_${axisIndex}`;
    formGroup.appendChild(inputDeadZone);

    // Champ : Invert avec switch Bootstrap (label avant input)
    const divInvert = document.createElement('div');
    divInvert.className = 'form-check form-switch';

    const labelInvert = document.createElement('label');
    labelInvert.className = 'form-check-label';
    labelInvert.textContent = 'Checked switch';
    divInvert.appendChild(labelInvert);

    const inputInvert = document.createElement('input');
    inputInvert.className = 'form-check-input';
    inputInvert.type = 'checkbox';
    inputInvert.id = `invert_${selectedDeviceIndex}_${axisIndex}`;
    inputInvert.checked = !!axis.invert;
    labelInvert.setAttribute('for', inputInvert.id);
    divInvert.appendChild(inputInvert);

    formGroup.appendChild(divInvert);

    // Champ : Virtual Joystick
    const labelVirtualJoystick = document.createElement('label');
    labelVirtualJoystick.textContent = 'Virtual Joystick:';
    formGroup.appendChild(labelVirtualJoystick);

    const inputVirtualJoystick = document.createElement('input');
    inputVirtualJoystick.className = 'form-control';
    inputVirtualJoystick.type = 'text';
    inputVirtualJoystick.value = axis.virtual_joystick || '';
    inputVirtualJoystick.id = `virtualJoystick_${selectedDeviceIndex}_${axisIndex}`;
    formGroup.appendChild(inputVirtualJoystick);

    // Champ : Virtual Axis
    const labelVirtualAxis = document.createElement('label');
    labelVirtualAxis.textContent = 'Virtual Axis:';
    formGroup.appendChild(labelVirtualAxis);

    const inputVirtualAxis = document.createElement('input');
    inputVirtualAxis.className = 'form-control';
    inputVirtualAxis.type = 'text';
    inputVirtualAxis.value = axis.mapped_axis || 0;
    inputVirtualAxis.id = `virtualAxis_${selectedDeviceIndex}_${axisIndex}`;
    formGroup.appendChild(inputVirtualAxis);

    currentRow.appendChild(collapseDiv);
  });
}

function generateButtonsUI() {
  // Cible le conteneur pour les boutons
  const container = document.getElementById('buttonsMappingContainer');
  if (!container) {
    console.error('Element with id "buttonsMappingContainer" not found.');
    return;
  }
  container.innerHTML = '';

  const device = globalMapping.devices[selectedDeviceIndex];
  if (!device) {
    container.innerHTML = '<p>Périphérique non trouvé</p>';
    return;
  }

  // On suppose que device.buttons est un objet avec des clés (0,1,2,3, etc.)
  const buttonKeys = device.buttons ? Object.keys(device.buttons).sort((a, b) => Number(a) - Number(b)) : [];
  if (buttonKeys.length === 0) {
    container.innerHTML = '<p>Aucun bouton disponible</p>';
    return;
  }

  let currentRow = null;
  buttonKeys.forEach((btnKey, index) => {
    const button = device.buttons[btnKey];

    // Créer une nouvelle ligne tous les 6 boutons
    if (index % 6 === 0) {
      currentRow = document.createElement('div');
      currentRow.className = 'row mt-4';
      container.appendChild(currentRow);
    }

    // Création du bloc de bouton collapsible
    const colDiv = document.createElement('div');
    colDiv.className = 'col-lg-2 col-md-8 col-12 mt-md-0';
    colDiv.setAttribute('data-bs-toggle', 'collapse');
    // Utilisation de la clé pour l'identifiant
    colDiv.setAttribute('href', `#buttonExample_${selectedDeviceIndex}_${btnKey}`);
    colDiv.setAttribute('role', 'button');
    colDiv.setAttribute('aria-expanded', 'false');
    colDiv.setAttribute('aria-controls', `collapseButton_${selectedDeviceIndex}_${btnKey}`);

    // Carte principale affichant la clé (nom du bouton)
    const card = document.createElement('div');
    card.className = 'card';

    const spanMask = document.createElement('span');
    spanMask.className = 'mask bg-dark opacity-10 border-radius-lg';
    card.appendChild(spanMask);

    const cardBody = document.createElement('div');
    cardBody.className = 'card-body p-3 position-relative';
    card.appendChild(cardBody);

    const innerRow = document.createElement('div');
    innerRow.className = 'row';
    cardBody.appendChild(innerRow);

    const col8 = document.createElement('div');
    col8.className = 'col-8 text-start';
    innerRow.appendChild(col8);

    const h5 = document.createElement('h5');
    h5.className = 'text-white font-weight-bolder mb-0 mt-3';
    // Affiche la clé comme nom du bouton
    h5.textContent = btnKey;
    col8.appendChild(h5);

    colDiv.appendChild(card);
    currentRow.appendChild(colDiv);

    // Création du contenu collapsible associé au bouton
    const collapseDiv = document.createElement('div');
    collapseDiv.className = 'collapse';
    collapseDiv.id = `buttonExample_${selectedDeviceIndex}_${btnKey}`;

    const collapseCard = document.createElement('div');
    collapseCard.className = 'card card-body';
    collapseDiv.appendChild(collapseCard);

    const form = document.createElement('form');
    collapseCard.appendChild(form);

    const formRow = document.createElement('div');
    formRow.className = 'row';
    form.appendChild(formRow);

    const colMd12 = document.createElement('div');
    colMd12.className = 'col-md-12';
    formRow.appendChild(colMd12);

    const formGroup = document.createElement('div');
    formGroup.className = 'form-group';
    colMd12.appendChild(formGroup);

    // Titre du collapse indiquant le nom du bouton (la clé)
    const h5Form = document.createElement('h5');
    h5Form.textContent = `Bouton : ${btnKey}`;
    formGroup.appendChild(h5Form);

    // Champ : Mapped Button
    const labelMappedButton = document.createElement('label');
    labelMappedButton.textContent = 'Mapped Button:';
    formGroup.appendChild(labelMappedButton);

    const inputMappedButton = document.createElement('input');
    inputMappedButton.className = 'form-control';
    inputMappedButton.type = 'text';
    inputMappedButton.value = button.mapped_button || 0;
    // Utilisation de la clé pour l'id
    inputMappedButton.id = `mappedButton_${selectedDeviceIndex}_${btnKey}`;
    formGroup.appendChild(inputMappedButton);

    // Champ : Virtual Button
    const labelVirtualButton = document.createElement('label');
    labelVirtualButton.textContent = 'Virtual Joystick:';
    formGroup.appendChild(labelVirtualButton);

    const inputVirtualButton = document.createElement('input');
    inputVirtualButton.className = 'form-control';
    inputVirtualButton.type = 'text';
    inputVirtualButton.value = button.virtual_joystick || 0;
    inputVirtualButton.id = `virtualJoystick_${selectedDeviceIndex}_${btnKey}`;
    formGroup.appendChild(inputVirtualButton);

    currentRow.appendChild(collapseDiv);
  });
}

document.getElementById('save-btn').addEventListener('click', async () => {
  // Mise à jour des données (axes et boutons) pour chaque périphérique
  globalMapping.devices.forEach((device, deviceIndex) => {
    // Mise à jour des axes
    if (device.axes && device.axes.length > 0) {
      device.axes.forEach((axis, axisIndex) => {
        const inputDeadZone = document.getElementById(`deadZone_${deviceIndex}_${axisIndex}`);
        const inputInvert = document.getElementById(`invert_${deviceIndex}_${axisIndex}`);
        const inputVirtualJoystick = document.getElementById(`virtualJoystick_${deviceIndex}_${axisIndex}`);
        const inputVirtualAxis = document.getElementById(`virtualAxis_${deviceIndex}_${axisIndex}`);
        if (inputDeadZone) axis.dead_zone = Number(inputDeadZone.value);
        if (inputInvert) axis.invert = inputInvert.checked;
        if (inputVirtualJoystick) axis.virtual_joystick = Number(inputVirtualJoystick.value);
        if (inputVirtualAxis) axis.mapped_axis = Number(inputVirtualAxis.value);
      });
    }
    // Mise à jour des boutons (en tant qu'objet)
    if (device.buttons) {
      Object.keys(device.buttons).forEach(btnKey => {
        const button = device.buttons[btnKey];
        const inputMappedButton = document.getElementById(`mappedButton_${deviceIndex}_${btnKey}`);
        const inputVirtualButton = document.getElementById(`virtualButton_${deviceIndex}_${btnKey}`);
        if (inputMappedButton) button.mapped_button = Number(inputMappedButton.value);
        if (inputVirtualButton) button.virtual_joystick = Number(inputVirtualButton.value);
      });
    }
  });
  
  // Reconstruction de l'objet mapping au format souhaité
  // Ici on utilise des valeurs par défaut pour global_axis_index et global_button_index si elles ne sont pas déjà définies.
  const mappingToSend = {
    global_axis_index: (globalMapping.global_axis_index !== undefined) ? globalMapping.global_axis_index : 99,
    global_button_index: (globalMapping.global_button_index !== undefined) ? globalMapping.global_button_index : 0,
    devices: globalMapping.devices
  };

  try {
    const response = await fetch('/mapping', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(mappingToSend, null, 2)
    });
    const text = await response.text();
    alert(text);
  } catch (err) {
    console.error('Erreur lors de la sauvegarde :', err);
  }
});


document.addEventListener('DOMContentLoaded', async () => {
  await loadMapping();
  renderDeviceTabs();
  renderSidebar();
});
