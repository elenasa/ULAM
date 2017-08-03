# Full rebuild everything from scratch target
rebuild:	FORCE
	@echo ======REALCLEAN
	@make realclean
	@echo MAKE
	@make
	@echo ======CLEANULAMEXPORTS
	@make cleanulamexports
	@echo ======ULAMEXPORTS
	@make ulamexports
	@echo ======DONE

.PHONY:	FORCE
