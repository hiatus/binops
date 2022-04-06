BINDIR := bin
TARGET := $(addprefix $(BINDIR)/, or and xor)

CWARNS := -Wall
CFLAGS := -std=gnu99 -Ofast

all: $(BINDIR) $(TARGET)

$(BINDIR)/or: main.c $(BINDIR)
	@echo [$(CC)] $@
	@$(CC) $(CWARNS) $(CFLAGS) -DBINOPS_OR -s -o $@ $<

$(BINDIR)/and: main.c $(BINDIR)
	@echo [$(CC)] $@
	@$(CC) $(CWARNS) $(CFLAGS) -DBINOPS_AND -s -o $@ $<

$(BINDIR)/xor: main.c $(BINDIR)
	@echo [$(CC)] $@
	@$(CC) $(CWARNS) $(CFLAGS) -DBINOPS_XOR -s -o $@ $<

$(BINDIR):
	@echo [mkdir] $(BINDIR)
	@mkdir $(BINDIR)

clean:
	@echo [rm] $(BINDIR)
	@rm -rf $(BINDIR) || true
