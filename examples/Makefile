# Copyright (c) 2026, Niklas Hauser
#
# This file is part of the modm project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Builds all examples, passing on variables like AVR_CXX and STD.

EXAMPLES := $(patsubst %/Makefile,%,$(wildcard */Makefile))

.PHONY: all clean $(EXAMPLES)
all: $(EXAMPLES)
$(EXAMPLES):
	@$(MAKE) --no-print-directory -C $@
clean:
	@for example in $(EXAMPLES); do $(MAKE) --no-print-directory -C $$example clean; done
