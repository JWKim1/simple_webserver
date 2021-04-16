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
