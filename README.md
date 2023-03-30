# Engine3D

Engine3D est une librairie permettant le chargement, la manipulation et le rendu de nuages de points avec un contrôle avancé des performances.
La librairie est disponible sur Windows, Ubuntu 22.04 et Android.

## Pré-requis
Télécharger [Visual Studio](https://visualstudio.microsoft.com/fr/) et installer :
* Développement Desktop en C++
* Kit de développement logiciel (SDK) Windows 11 (10.0.22621.0) -> Dans l'onglet "Composants individuels"

## Dépendances

* [Qt6](https://www.qt.io/)
* [glm](https://github.com/g-truc/glm)

### Optionnelles
* [OpenCV](https://opencv.org/)
* [ViSP](https://github.com/lagadic/visp) ([Eigen3](https://gitlab.com/libeigen/eigen) et [OpenCV](https://opencv.org/) sont nécessaires)
* [OpenVR](https://github.com/ValveSoftware/openvr)

## Compilation

### Windows

#### Qt
Télécharger l'[installeur de Qt](https://www.qt.io/download-qt-installer) sur le site officiel et installer la dernière version de Qt6 avec le compilateur **MSVC**.

#### glm
```
git clone https://github.com/g-truc/glm
cd glm && mkdir build && cd build
cmake -DBUILD_TESTING=False -DCMAKE_INSTALL_PREFIX=install ..
cmake --build . --config Release --target install
```

#### OpenCV

##### Librairie pré-compilée
Télécharger et extraire la [dernière version d'OpenCV](https://opencv.org/releases/) sur le site officiel.

#### ViSP

La librairie ViSP a besoin d'être compilée avec deux autres librairies : Eigen (pour [libPeR](https://github.com/PerceptionRobotique/libPeR)) et OpenCV (pour [ESILab](https://github.com/PerceptionRobotique/ESILab)).

##### Eigen
```
git clone https://gitlab.com/libeigen/eigen.git
cd eigen && mkdir build && cd build
cmake -DBUILD_TESTING=FALSE -DCMAKE_INSTALL_PREFIX=install ..
cmake --build . --config Release --target install
```

##### OpenCV
Il est recommandé d'utiliser la version release pré-compilée d'OpenCV (plus simple et Engine3D a été testé sur celle-ci). Pour utiliser OpenCV depuis les sources, il faut également ajouter [opencv_contrib](https://github.com/opencv/opencv_contrib) et donner le dossier "modules" dans la variable CMake.
```
git clone https://github.com/opencv/opencv_contrib.git
git clone https://github.com/opencv/opencv.git
cd opencv
mkdir build && cd build

$OpenCV_CONTRIB_MODULES_DIR="../../opencv_contrib/modules"

$OpenCV_CONTRIB_MODULES_DIR=$OpenCV_CONTRIB_MODULES_DIR.replace("\","/")

cmake -DBUILD_PERF_TESTS=False -DBUILD_TESTS=False -DBUILD_opencv_python_tests=False -DOPENCV_EXTRA_MODULES_PATH="$OpenCV_CONTRIB_MODULES_DIR" -DBUILD_opencv_world=True ..
cmake --build . --config Release --target install
```

##### ViSP
Le dépôt officiel de ViSP ne fonctionne pas avec Engine3D à cause d'une incompatibilité avec PThread même en le désactivant. Il faut utiliser un fork corrigeant vpImage et vpImageConvert.
```
git clone https://github.com/NoelVillette/visp.git
cd visp
git checkout remotes/origin/patch4engine3d -b patch4engine3d
mkdir build && cd build

$Eigen3_DIR="<Eigen3_INSTALL_DIR>"
$OpenCV_DIR="<OpenCV_BUILD_DIRECTORY>"

$Eigen3_DIR=$Eigen3_DIR.replace("\","/")
$OpenCV_DIR=$OpenCV_DIR.replace("\","/")

cmake -DBUILD_DEMOS=FALSE -DBUILD_EXAMPLES=FALSE -DBUILD_TESTS=FALSE -DBUILD_TUTORIALS=FALSE -DEIGEN3_INCLUDE_DIR="$Eigen3_DIR/include/eigen3" -DOpenCV_DIR="$OpenCV_DIR" -DWITH_PTHREAD=FALSE ..
cmake --build . --config Release --parallel --target install
```

#### OpenVR
```
git clone https://github.com/ValveSoftware/openvr.git
```

#### Engine3D
```
git clone https://github.com/PerceptionRobotique/Engine3D.git
cd Engine3D
mkdir build && cd build

$QT_DIR="<QT_MSVC_DIR>"
$glm_DIR="<glm_INSTALL_DIRECTORY>"
$OpenCV_DIR="<OpenCV_BUILD_DIRECTORY>"
$ViSP_DIR="<ViSP_INSTALL_DIRECTORY>"
$OpenVR_DIR="<OpenVR_DIRECTORY>"

$QT_DIR=$QT_DIR.replace("\","/")
$glm_DIR=$glm_DIR.replace("\","/")
$OpenCV_DIR=$OpenCV_DIR.replace("\","/")
$ViSP_DIR=$ViSP_DIR.replace("\","/")
$OpenVR_DIR=$OpenVR_DIR.replace("\","/")

cmake -DQT_DIR="$QT_DIR/lib/cmake/Qt6" -Dglm_DIR="$glm_DIR/lib/cmake/glm" -DWITH_OPENCV=TRUE -DOpenCV_DIR="$OpenCV_DIR" -DWITH_ViSP=TRUE -DVISP_DIR="$ViSP_DIR" -DWITH_OpenVR=TRUE -DOPENVR_DIR="$OpenVR_DIR" ..
cmake --build . --config Release --parallel --target install
```

##### Installer en Debug

Compiler et installer ViSP et Engine3D en configuration Debug :
```
cmake --build . --config Debug --target install
```

##### Créer un package
Installer [NSIS](https://nsis.sourceforge.io/Download). Si Engine3D a été compilé et installé en Debug et Release alors les deux configurations seront présentes dans le package.

`cmake --build . --config Release --target package`

### Ubuntu

#### Installer les dépendances
`sudo apt install git cmake build-essential ninja-build ffmpeg libgl1-mesa-dev libglu1-mesa-dev`

#### Installation des librairies depuis les dépôts Ubuntu
`sudo apt install qt6-base-dev libglm-dev libopencv-dev libvisp-dev libopenvr-dev`

#### Installation des librairies depuis les sources

Cette section est à titre d'information. Il est recommandé de passer par les dépôts officiels de Ubuntu. Cela peut-être utile par exemple sur d'anciennes version d'Ubuntu où certains dépôts manquent, mais la compatibilité n'est pas garantie.

##### Qt6 (non recommandé)
Il est tout à fait possible d'utiliser Qt6 depuis [l'installeur](https://www.qt.io/download-qt-installer) disponible sur leur site internet.
```
sudo apt install libfontconfig1-dev libfreetype6-dev libx11-dev libx11-xcb-dev libxext-dev libxfixes-dev libxi-dev libxrender-dev libxcb1-dev libxcb-glx0-dev libxcb-keysyms1-dev libxcb-image0-dev libxcb-shm0-dev libxcb-icccm4-dev libxcb-sync-dev libxcb-xfixes0-dev libxcb-shape0-dev libxcb-randr0-dev libxcb-render-util0-dev libxcb-util-dev libxcb-xinerama0-dev libxcb-xkb-dev libxkbcommon-dev libxkbcommon-x11-dev
git clone https://github.com/qt/qtbase.git
cd qtbase
mkdir build && cd build
../configure -release -shared
cmake --build . --parallel
sudo cmake --build . --target install
```

Si Qt6 a été compilé en `shared`, ajouter le dossier des librairies à la variable d'environnement PATH. Pour cela, ajouter à la fin du fichier `.bashrc` :
```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/Qt-6.<version>/lib:/usr/local/Qt-6.<version>/plugins
```

##### glm
```
git clone https://github.com/g-truc/glm.git
cd glm
mkdir build && cd build
cmake -DBUILD_TESTING=False ..
sudo cmake --build . --config Release --target install
```

##### Eigen
```
git clone https://gitlab.com/libeigen/eigen.git
cd eigen
mkdir build && cd build
cmake -DBUILD_TESTING=False ..
sudo cmake --build . --config Release --target install
```

##### OpenCV
```
git clone https://github.com/opencv/opencv.git
git clone https://github.com/opencv/opencv_contrib.git
cd opencv
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=False -DBUILD_PERF_TESTS=False -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules -DWITH_EIGEN=False ..
cmake --build . --config Release --parallel
sudo cmake --build . --config Release --target install
```

##### ViSP
```
git clone https://github.com/lagadic/visp.git
cd visp
mkdir build && cd build
cmake -DBUILD_DEMOS=False -DBUILD_EXAMPLES=False -DBUILD_TESTS=False -DBUILD_TUTORIALS=False -DEIGEN3_INCLUDE_DIR=/usr/local/include/eigen3 ..
cmake --build . --config Release --parallel
sudo cmake --build . --config Release --target install
```

##### OpenVR
```
git clone https://github.com/ValveSoftware/openvr.git
cd openvr
mkdir build && cd build
cmake ..
cmake --build . --parallel
sudo cmake --build . --target install
```

#### Compiler Engine3D
```
git clone https://github.com/PerceptionRobotique/Engine3D.git
cd Engine3D
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DWITH_OPENCV=TRUE -DWITH_ViSP=TRUE -DWITH_OpenVR=TRUE ..
cmake --build . --config Release --parallel
```

#### Compiler en Debug
```
cmake -DCMAKE_BUILD_TYPE=Debug -DWITH_OPENCV=TRUE -DWITH_ViSP=TRUE -DWITH_OpenVR=TRUE ..
cmake --build . --config Debug --parallel
```

#### Création d'un package DEB
Si `libEngine3Dd.so` existe dans le dossier de build alors le package va inclure les deux configurations.
```
cmake -DCMAKE_BUILD_TYPE=Release -DWITH_OPENCV=TRUE -DWITH_ViSP=TRUE -DWITH_OpenVR=TRUE ..
cmake --build . --config Release --target package
```

### Android

Les étapes d'installation et de compilation pour Android sont très complexes et demandent parfois de faire face à des comportements "étranges" surtout venant de Qt. Les étapes ont été réalisées sur Windows.

#### Installer et configurer les outils

* Télécharger et installer la dernière version de [Qt6]{https://www.qt.io/} pour Android
* Télécharger et installer [Java JDK]{https://download.oracle.com/java/17/latest/jdk-17_windows-x64_bin.exe} version 17 (LTS) pour éviter certains problèmes
* Télécharger et installer [Android Studio]{https://developer.android.com/studio/}
* Lancer Android Studio pour qu'il installe les outils par défaut
* Dans le launcher d'Android Studio, aller dans `SDK Manager`
* Cocher la dernière version du SDK d'Android (avec un "API Level" correspondant à un numéro, actuellement Android 13.0 Tiramisu avec un API Level à 33)
* Aller dans `More Actions > SDK Manager > SDK Tools`
* Décocher `Hide Obsolete Packages`
* Cocher `Android SDK Command-line Tools (latest)`
* Cocher `Android SDK Tools (Obsolete)` car il sera nécessaire pour ViSP
* Valider l'installation et fermer Android Studio
* Lancer Qt Creator
* Aller dans `Édition > Preferences... > Appareils mobiles > Android`
* Remplir l'emplacement du JDK (installé dans `C:\Program Files\Java\jdk-17`)
* Si Qt demande d'installer des packages supplémentaires répondre oui
* Remplir l'emplacement du SDK Android si besoin (par défaut `C:\Users\<user>\AppData\Local\Android\Sdk`)
* Cliquer sur `Setup SDK` si Android n'est pas encore configuré, il faut parfois cliquer plusieurs fois dessus pour avoir toutes les coches vertes
* Si OpenSSL est manquant, aller dans `C:\Users\<user>\AppData\Local\Android\Sdk`, ouvrir un terminal et cloner le dépôt git d'Android OpenSSL : `git clone https://github.com/KDAB/android_openssl.git`
* Redémarrer Qt Creator et vérifier qu'Android OpenSSL est OK.

Normalement, toutes les coches sont au vert.

#### Compilation

Pour rappel, les libraires OpenCV et ViSP sont optionnelles.

##### glm

* `git clone https://github.com/g-truc/glm.git`
* Ouvrir le fichier `CMakeLists.txt` de glm dans Qt
* glm peut être configuré pour Android sur les architectures arm64-v8a et x86_64 (sélectionner les configurations "Release" sauf si vous savez ce que vous faites)
* Le changement de configuration se fait en cliquant sur le bouton au dessus de "Run" en bas à gauche (logo de smartphone avec une pastille rouge)
* Aller dans `Projets`
* Attendre la fin de la configuration initiale de glm
* Aller dans l'onglet `Current Configuration`
* Décocher les options : `BUILD_TESTING`
* Changer `CMAKE_INSTALL_PREFIX` pour "install"
* Dans "Étapes Build", cliquer sur "Détails" de Compilation, décocher `all` et cocher `install`
* Désactiver le "Build Android APK" (logo à sélectionner à côté de "Détails")
* Cliquer sur "Exécuter CMake" et attendre la fin de la configuration
* Compiler (logo du marteau en bas à gauche)
* Recommencer avec les autres architectures de processeur voulues

##### OpenCV

Télécharger et extraire la librairie [OpenCV]{https://opencv.org/releases/} pour Android.

##### ViSP

* `git clone https://github.com/lagadic/visp.git`
* Ouvrir le fichier `CMakeLists.txt` de ViSP dans Qt
* ViSP peut être configuré pour Android sur les architectures arm64-v8a et x86_64 (sélectionner les configurations "Release" sauf si vous savez ce que vous faites)
* Le changement de configuration se fait en cliquant sur le bouton au dessus de "Run" en bas à gauche (logo de smartphone avec une pastille rouge)
* Aller dans `Projets`
* Attendre la fin de la configuration initiale de ViSP
* Aller dans l'onglet `Current Configuration`
* Décocher les options : `BUILD_ANDROID_EXAMPLES`, `BUILD_ANDROID_PROJECTS`, `BUILD_DEMOS`, `BUILD_EXAMPLES`, `BUILD_JAVA`, `BUILD_TESTS` et `BUILD_TUTORIALS`
* Cocher l'option `BUILD_SHARED_LIBS`
* Dans "Étapes Build", cliquer sur "Détails" de Compilation, décocher `all` et cocher `install`
* Désactiver le "Build Android APK" (logo à sélectionner à côté de "Détails")
* Cliquer sur "Exécuter CMake" et attendre la fin de la configuration (vérifier dans la sortie que Tests, Demos, Examples et Tutorials retournent "no", sinon fermer Qt Creator, supprimer tous les fichiers dans le dossier de build en cours à l'exception du "CMakeCache.txt" et recommencer, la configuration est normalement sauvegardée donc il suffit de ré-exécuter CMake)
* Compiler (logo du marteau en bas à gauche)
* Recommencer avec les autres architectures de processeur voulues

##### Engine3D

* `git clone https://github.com/PerceptionRobotique/Engine3D.git`
* Ouvrir le fichier `CMakeLists.txt` d'Engine3D dans Qt
* Engine3D peut être configuré pour Android sur les architectures arm64-v8a et x86_64 (sélectionner les configurations "Release" sauf si vous savez ce que vous faites)
* Aller dans `Projets`
* Attendre la fin de la configuration initiale (qui échoue, c'est normal)
* Aller dans l'onglet `Current Configuration`
* Créer la variable suivante en cliquant sur `Add > Directory` :
	* `glm_DIR` : `<glm_install_dir>/lib/cmake/glm`
* Cliquer sur "Exécuter CMake"
* Définir la variable : `CMAKE_INSTALL_PREFIX` : `install`
* [optionnel] Cocher les variables : `WITH_OPENCV` et `WITH_ViSP` et définir les variables :
	* `OpenCV_DIR` : `<chemin_vers_OpenCV_android>/OpenCV-android-sdk/sdk/native/jni`
	* `VISP_DIR` : `<visp_install_dir>/sdk/native/jni`
* Changer `CMAKE_INSTALL_PREFIX` pour "install"
* Dans "Étapes Build", cliquer sur "Détails" de Compilation, décocher `all` et cocher `install`
* Désactiver le "Build Android APK" (logo à sélectionner à côté de "Détails")
* Cliquer sur "Exécuter CMake" et attendre la fin de la configuration (vérifier dans la sortie que Tests, Demos, Examples et Tutorials retournent "no", sinon fermer Qt Creator, supprimer tous les fichiers dans le dossier de build en cours à l'exception du "CMakeCache.txt" et recommencer, la configuration est normalement sauvegardée donc il suffit de ré-exécuter CMake)
* Compiler (logo du marteau en bas à gauche)
* Recommencer avec les autres architectures de processeur voulues

### Documentation

Pour compiler la documentation, installer Doxygen.

* Windows : https://www.doxygen.nl/download.html
* Ubuntu : `sudo apt install doxygen`

Ajouter `-DGENERATE_DOXYGEN=TRUE` à la commande cmake. Exécuter ensuite `cmake --build . --target Doxygen`. Quand la documentation a été générée, elle est installée avec le package.
