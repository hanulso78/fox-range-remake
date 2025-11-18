1. allegro 4.2.2 sound blaster 버그 패치
출처: https://www.allegro.cc/forums/thread/596505/753496#target
all422s.zip allegro 4.2.2 버전의 소스를 압축을 풀고
패치된 sb.c 파일을 allegro\src\dos 에 복사한다.

2. 패치2번째
allegro-4.2.2-binutils-2.18.50.patch 
파일의 내용대로 수정한다.

빌드는 반드시 리얼도스에서 한다.
fix.bat djgpp
make UNIX_TOOLS=0
