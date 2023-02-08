# Engine3D

Engine3D est une librairie permettant le chargement, la manipulation et le rendu de nuages de points avec un contrôle avancé des performances.
La librairie est disponible sur Windows, Ubuntu 22.04 et Android.

## Pré-requis
Télécharger [Visual Studio](https://visualstudio.microsoft.com/fr/) et installer "Développement Desktop en C++".

## Dépendances

* [Qt6](https://www.qt.io/)
* [glm](https://github.com/g-truc/glm)

Optionnelles
* [OpenCV](https://opencv.org/)
* [ViSP](https://github.com/lagadic/visp) (OpenCV et [Eigen3](https://gitlab.com/libeigen/eigen) sont nécessaires)
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
cmake --build . --config=Release --target install
```

#### OpenCV

##### Librairie pré-compilée
Télécharger et extraire la [dernière version d'OpenCV](https://opencv.org/releases/) sur le site officiel.

#### ViSP

##### Compilation manuelle

###### Eigen
```
git clone https://gitlab.com/libeigen/eigen.git
cd eigen && mkdir build && cd build
cmake -DBUILD_TESTING=FALSE -DCMAKE_INSTALL_PREFIX=install ..
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
cmake --build . --config Release --target install
```

#### OpenVR
```
git clone https://github.com/ValveSoftware/openvr.git
```

#### Engine3D
```
git clone https://github.com/PerceptionRobotique/Engine3D.git
cd Engine3D && mkdir build && cd build

$glm_DIR="<glm_INSTALL_DIRECTORY>"
$OpenCV_DIR="<OpenCV_BUILD_DIRECTORY>"
$ViSP_DIR="<ViSP_INSTALL_DIRECTORY>"
$OpenVR_DIR="<OpenVR_DIRECTORY>"

$glm_DIR=$glm_DIR.replace("\","/")
$OpenCV_DIR=$OpenCV_DIR.replace("\","/")
$ViSP_DIR=$ViSP_DIR.replace("\","/")
$OpenVR_DIR=$OpenVR_DIR.replace("\","/")

cmake -Dglm_DIR="$glm_DIR/lib/cmake/glm" -DWITH_OPENCV=TRUE -DOpenCV_DIR="$OpenCV_DIR" -DWITH_ViSP=TRUE -DVISP_DIR="$ViSP_DIR" -DWITH_OpenVR=TRUE -DOPENVR_DIR="$OpenVR_DIR" ..
cmake --build . --config Release --target install
```

##### Installer en Debug

Compiler et installer ViSP et Engine3D en configuration Debug :
```
cmake --build . --config Debug --target install
```

##### Créer un package
Installer [NSIS](https://nsis.sourceforge.io/Download). Si Engine3D a été compilé et installé en Debug et Release alors les deux configurations seront présentes dans le package.

`cmake --build . --config Release --target package`
