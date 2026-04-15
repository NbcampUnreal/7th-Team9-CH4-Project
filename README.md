# 7th-Team9-CH4-Project

## 프로젝트 개요
본 프로젝트는 언리얼 엔진 5를 사용하여 개발된 4인 협동 서바이벌 호러 게임입니다.
플레이어는 맵 곳곳에 흩어진 팝콘을 수집하여 팝콘 기계를 완성하고, 팝콘을 튀겨 탈출해야 합니다.

## 주요 기능

### 1. 멀티플레이어 시스템
- **Steam Integration**: Steamworks SDK를 활용한 온라인 멀티플레이어 지원
- **Session Management**: 세션 생성, 참여, 관리 기능
- **Replication**: 캐릭터 이동, 상호작용, 게임 상태 등의 실시간 복제

### 2. 게임 플레이
- **Popcorn Collection**: 맵 곳곳에 흩어진 팝콘 수집
- **Popcorn Machine**: 수집한 팝콘으로 팝콘 기계 완성
- **Popcorn Popping**: 완성된 기계로 팝콘 튀기기
- **Escape**: 팝콘을 튀겨 탈출

### 3. 캐릭터 시스템
- **4-Player Co-op**: 최대 4명의 플레이어가 협동 플레이
- **Custom Name**: 플레이어 이름 설정 기능
- **Ready System**: 게임 시작 전 준비 상태 확인
- **Character Movement**: 이동, 점프, 앉기 등 기본 액션

### 4. 게임 모드
- **Lobby System**: 게임 시작 전 대기 공간
- **Game Mode**: 게임 진행 로직
- **Game State**: 게임 상태 관리
- **Player State**: 플레이어 상태 관리

## 개발 환경
- **Engine**: Unreal Engine 5.6.2
- **Language**: C++
- **Platform**: Windows

## 프로젝트 구조
```
FarmsPopcorn/
├── Content/
│   ├── Maps/              # 게임 맵
│   ├── Blueprints/        # 블루프린트 에셋
│   ├── Materials/         # 머티리얼 에셋
│   ├── Meshes/            # 메쉬 에셋
│   └── ...
├── Source/
│   ├── FarmsPopcorn/      # 게임 모듈
│   │   ├── Public/        # 헤더 파일
│   │   └── Private/       # 소스 파일
│   ├── ThirdParty/        # 외부 라이브러리
│   └── ...
├── Config/                # 설정 파일
├── Plugins/               # 플러그인
└── ...
```

## 빌드 및 실행
1. **Unreal Engine 5.6.2** 설치
2. 프로젝트 폴더 열기
3. **FarmsPopcorn.uproject** 파일 실행
4. **Edit -> Plugins**에서 **Steamworks** 플러그인 활성화
5. **Edit -> Project Settings -> Maps & Modes**에서 기본 맵 설정
6. **Play** 버튼 클릭하여 게임 실행

## 사용 방법
1. **Host** 버튼 클릭하여 게임 서버 생성
2. **Join** 버튼 클릭하여 친구 게임 참여
3. **Character Name** 입력
4. **Ready** 버튼 클릭
5. 모든 플레이어가 준비되면 게임 시작

## 게임 플레이 방법
1. 맵 곳곳에 흩어진 **팝콘** 수집
2. 수집한 팝콘으로 **팝콘 기계** 완성
3. 완성된 기계에서 **팝콘 튀기기**
4. 튀겨진 팝콘을 가지고 **탈출 지점**으로 이동
5. **탈출**하여 게임 승리

## 개발 참고
- **Steamworks SDK**: [https://partner.steamgames.com/doc/sdk](https://partner.steamgames.com/doc/sdk)
- **Unreal Engine Documentation**: [https://docs.unrealengine.com/](https://docs.unrealengine.com/)
- **Unreal Engine C++ API**: [https://api.unrealengine.com/](https://api.unrealengine.com/)

## 라이선스
본 프로젝트는 MIT 라이선스를 따릅니다.
자세한 내용은 [LICENSE](LICENSE) 파일을 참조하세요.