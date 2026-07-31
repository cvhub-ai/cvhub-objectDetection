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
│ │<br>
│ ├── core/ <br>
│ │ ├── engine.cpp<br>
│ │ ├── engine.h<br>
│ │ ├── processor.cpp<br>
│ │ └── processor.h<br>
│ │<br>
│ ├── pipline/ <br>
│ │ ├── preprocessing/<br>
│ │ │ ├── preprocessing.cpp<br>
│ │ │ └── preprocessing.h<br>
│ │ ├── recognition<br>
│ │ │ ├── recognition.cpp<br>
│ │ │ └── recognition.h<br>
│ │ ├── postprocessing<br>
│ │ │ ├── postprocessor.cpp<br>
│ │ │ └── postprocessor.h<br>
│ │ ├── rendering<br>
│ │ │ ├── renderer.cpp<br>
│ │ │ └── renderer.h<br>
│ │ ├── display<br>
│ │ │ ├── display.cpp<br>
│ │ │ └── display.h<br>
│ │<br>
│ ├── utils/ <br>
│ │ ├── logger.cpp<br>
│ │ ├── logger.h<br>
│ │ ├── string_utils.cpp<br>
│ │ ├── string_utils.h<br>
│ ├── interfaces/ <br>
│ │ ├── pipeline_stage.h<br>
│ │ ├── result_callback.h.h<br>
│ │ └── algorithm_factory.h<br>
│<br>
├── include/ <br>
│ ├── api.h <br>
│ ├── config.h<br>
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
