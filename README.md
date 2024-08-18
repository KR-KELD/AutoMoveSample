# AutoMoveSample

이 프로젝트는 언리얼 엔진을 사용하여 개발된 자동이동 샘플 프로젝트입니다


## 프로젝트 소개

PlayerController에서 특정 Pawn을 빙의하는 중에 AIController 서브 컨트롤러로 사용해 Behavior Tree를 사용할 수 있게 구현하였습니다.

1. 독자적으로 연결된 AIController를 사용해 Behavior Tree를 실행시켜 자동이동을 실행합니다.
2. 자동이동 TaskNode가 실행중일 때 남은 이동 경로를 표시해줍니다.
3. 자동이동중 플레이어 인풋을 감지하면 자동이동을 정지합니다.

## 설치 및 실행 방법
1. 프로젝트를 클론합니다.
2. PlayerAISample.uproject파일을 우클릭하여 Generate Visual Studio project files를 누릅니다.
3. PlayerAISample.sln 파일을 실행하여 Development Editor옵션을 선택 후 프로젝트를 컴파일 해줍니다. 
4. UE5 프로젝트를 실행합니다.

## 키 설명
1. Q키 : 자동이동 시작
2. E키 : 자동이동 종료
