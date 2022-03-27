# name of your application
APPLICATION = Project_OS

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# This defaults to build with round_robin using ztimer
RR ?= 1

USEMODULE += ztimer_usec

ifeq (1,$(RR))
  USEMODULE += sched_round_robin
endif

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include