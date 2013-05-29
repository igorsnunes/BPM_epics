TOP=../..

include $(TOP)/configure/CONFIG
#----------------------------------------
#  ADD MACRO DEFINITIONS AFTER THIS LINE
#=============================

#=============================
# Build the IOC application

PROD_IOC = BPM
# BPM.dbd will be created and installed
DBD += BPM.dbd

# BPM.dbd will be made up from these files:
BPM_DBD += base.dbd
BPM_DBD += LPCdev.dbd

# Include dbd files from all support applications:
#BPM_DBD += xxx.dbd

# Add all the support libraries needed by this IOC
#BPM_LIBS += xxx

# BPM_registerRecordDeviceDriver.cpp derives from BPM.dbd
BPM_SRCS += BPM_registerRecordDeviceDriver.cpp
BPM_SRCS += devLPCAi.c
BPM_SRCS += epicsTCP.c
BPM_SRCS += ../../Api-para-Protocolo-de-Controle-de-Baixo-n-vel/pcbnAPI.c

# Build the main IOC entry point on workstation OSs.
BPM_SRCS_DEFAULT += BPMMain.cpp
BPM_SRCS_vxWorks += -nil-

# Add support from base/src/vxWorks if needed
#BPM_OBJS_vxWorks += $(EPICS_BASE_BIN)/vxComLibrary

# Finally link to the EPICS Base libraries
BPM_LIBS += $(EPICS_BASE_IOC_LIBS)
#===========================

include $(TOP)/configure/RULES
#----------------------------------------
#  ADD RULES AFTER THIS LINE

