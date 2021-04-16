# simple_webserver
simple webserver

-간단한 웹 서버 만들기

-multiple clinet의 요청 동시 처리 가능

-웹서버 시작시 사용할 service directory와 port를 parameter로 받음.

-total.cgi 처리
  total.cgi?from=NNN&to=MMM을 요청할 경우 NNN~MMM까지의 총합을 구해서 그 결과를 보여줌.
  (NNN, MMM은 숫자형태)
  
  -http_load-test를 사용하여 성능 평가
    성능 향상을 위한 기능 추가



주어진 이미지 파일을 사용하여 조건에 맞는 간단한 웹서버를 제작하고,
cgi 요청시 간단한 연산을 웹에 표시하도록 하였다. 
또한 웹 서버의 성능 향상을 위한 알고리즘을 추가하고
그에 따른 효율을 비교분석하는 프로젝트를 진행하였다.

서버에 널리 사용되는 유닉스 개발환경을 바탕으로 프로그램을 개발하였다.
pc의 웹브라우저로 지정 명령어 요청 시
요청된 파일에 대한 전달을 수행 표시하도록 하였다.
또한 특별한 웹페이지인 ‘total.cgi’를 임의의 변수 두 개와 함께 요청 시
두 변수 사이의 숫자의 총합을 구하여 결과를 보여주도록 하였다.

메인 파일에서 모든 기능을 수행하였고,
유닉스 환경에서 용이한 컴파일 및 수행을 위해 makefile을 이용하여
make, make clean 등의 명령어를 통해 사용자가 실행하기 용이하도록 설계하였다.

지정된 명령어를 파악하여 변수에 저장하고
이를 바탕으로 파일을 불러와 웹에 표시하도록 하고, 
total.cgi 명령어의 경우
연산을 위해 숫자를 변수에 저장 연산 후 웹에 프린트하도록 하며
변수가 없을 경우 에러 메시지를 프린트하도록 하였다.

또한 성능 향상을 위한 기능도 추가하였다 먼저 Nagle 알고리즘을 비활성화 하였다.
이는 delay 를 줄여 속도 향상에 도움을 준다.
또한 pthread를 이용,
스레드를 분리하여 작업이 동시에 작동할 수 있도록 하여 실행 시간을 줄였다.
이를 비교하기 위해 응답시간을 프로그램 내에서 가져와 도표를 작성하였고,
각 기능이 어느 정도의 성능향상 효과를 가지는지 분석하였다.

