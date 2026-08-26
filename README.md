# Objectdetection

## Introduction

## Structure
cvhub-objectDetection/ <br> 
├── CMakeLists.txt <br>
├── README.md <br>
├── LICENSE <br>
├── .gitignore <br>
│ <br>
├── src/ <br>
│ ├── main.cpp <br>
│ └── inference.cpp <br>
│ │<br>
│ ├── core/ <br>
│ │ ├── engine.cpp<br>
│ │ ├── engine.h<br>
│ │ ├── processor.cpp<br>
│ │ └── processor.h<br>
│ │<br>
│ ├── utils/ <br>
│ │ ├── cli.cpp<br>
│ │ ├── draw.hpp<br>
│ │ ├── render.hpp<br>
│ │ ├── show.hpp<br>
│ │ ├── logger.hpp<br>
│ │ └── logger.h<br>
│<br>
├── include/ <br>
│ ├── pipline/ <br>
│ │ ├── preprocess.hpp <br>
│ │ └── inference.h<br>
│ ├── coco_names.hpp <br>
│ ├── types.hpp <br>
│ └── version.h<br>
│<br>
├── tests/ <br>
│ ├── unit/ <br>
│ │ ├── test_engine.cpp<br>
│ │ ├── test_pipline.cpp<br>
│ │ └── test_utils.cpp<br>
│ ├── integration/ <br>
│ │ └── test_integration.cpp<br>
│<br>
├── build/ <br>
│ ├── Debug/<br>
│ └── Release/<br>
│<br>
├── bin/ <br>
│ ├── objectdetection<br>
│ └── test-objectdetection<br>
│<br>
├── models/ <br>
│<br>
├── scripts/ <br>
│ ├── build.sh<br>
│ ├── run_tests.sh<br>
│ └── generate_docs.sh<br>
│<br>
├── docker/ <br>
│ ├── Dockerfile<br>
│ └── docker-compose.yml<br>
│<br>
└── .vscode/ <br>
├── launch.json<br>
└── settings.json<br>
