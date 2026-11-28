# ZARC 빠른 시작 (한국어)

번역 상태: 초안입니다. 한국어 기술 검토와 파트너 승인은 아직 완료되지 않았습니다.

ZARC는 Zephyr 기반 자동차 제어 및 통합 프레임워크이며 이식 가능한 C17 코어를 포함합니다. 저장소에는 데스크톱, 호스트 서비스, Zephyr 네이티브 시뮬레이터 참고 구현이 있습니다. 이는 실제 하드웨어 지원이나 제품 자격을 의미하지 않습니다. VS Code에서 `ZARC.code-workspace`를 여세요. CMake 3.21 이상과 C17 컴파일러가 필요합니다.

지원되는 개발 설정의 영어 원문은 [README 빠른 시작](../../README.md#quick-start)을 참조하세요. 용어는 [다국어 용어집](GLOSSARY.md), 번역 및 검토 절차는 [워크플로](WORKFLOW.md)를 참조하세요.

## Windows

```powershell
cmake --preset desktop-windows
cmake --build --preset desktop-windows
ctest --preset desktop-windows
```

## Linux

```bash
cmake --preset desktop-linux
cmake --build --preset desktop-linux
ctest --preset desktop-linux
```

호스트 운영 체제에 맞는 preset을 사용하세요. Windows에서는 MSVC가 필요하며 셸에서 CMake를 사용할 수 있어야 합니다. Linux에서는 Ninja와 GCC 또는 Clang이 필요합니다. 운영 체제별로 별도의 빌드 디렉터리를 사용하세요.

현재 구현 범위와 미완료 항목은 [구현 상태](../IMPLEMENTATION-STATUS.md)를 참조하세요. 이 빠른 시작은 데스크톱 시뮬레이션을 실행합니다. 실제 하드웨어 동작, 제품 지원 또는 독립 안전 자격을 입증하지 않습니다.
