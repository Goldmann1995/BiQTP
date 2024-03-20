##################################################
###              Makefile of BiQTP             ###
##################################################

#####   all   #####
# all-默认执行动作
all: buildproj


#####   QTP_MACRO   #####
QTP_SRC = lib/inih/ini.c lib/inih/INIReader.cpp \
		  include/ThreadBase.cpp \
		  src/MDRing.cpp src/MDReceiver.cpp src/Calculator.cpp src/Strategy.cpp src/StrategyBOX.cpp \
		  src/BiIniter.cpp src/BiQTP.cpp
INCLUDE_SRC = -Iinclude -Ilib
LIB_SRC = -lcurl -lcrypto


EXTRAITEM = bin/BiQTP
#CC = g++ -std=c++11 -Wall -mcmodel=medium
CC = g++ -std=c++11 -Wall
AR = ar


#####   buildproj   #####
buildproj:
#ZMate_Proj: src/ThreadBase.cpp src/Controller.cpp src/UniThread.cpp src/ZLogger.cpp src/MDQReplayer.cpp src/ZETA_QTS.cpp
	@echo -e "\033[33mbuild proj\033[0m"
#	@rm -f $@
#	$(CC) $(ZETA_SRC) $(INCLUDE_SRC) $(LIB_SRC) -o bin/$@
	$(CC) $(QTP_SRC) $(INCLUDE_SRC) $(LIB_SRC) -o $(EXTRAITEM)


#####   clean   #####
clean:
	@echo -e "\033[33mclean\033[0m"
	@rm -f $(EXTRAITEM)


#####   test   #####
test:
	echo "hello summer"
	@echo -e "\033[33mhello summer\033[0m"
