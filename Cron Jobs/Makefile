include ../Makefile.inc
SOHBASEDIR=$(BASEDIR)

.PHONY: clean

reboot_proccheck.cron: reboot_proccheck.cron.glob reboot.cron SOHProcCheck.cron
	@cp $< $@
	@sed -i 's#%SOHBASEDIR%#$(SOHBASEDIR)#' $@
	@sed -i 's#%MIDASSYS%#$(MIDASSYS)#' $@
	@sed -i 's#%MIDAS_EXPTAB%#$(MIDAS_EXPTAB)#' $@
	@sed -i 's#%MIDAS_EXPT_NAME%#$(MIDAS_EXPT_NAME)#' $@
	@sed -i 's#%REBOOT_CRON%#$(shell cat reboot.cron)#' $@
	@sed -i 's#%PROCCHECK_CRON%#$(shell cat SOHProcCheck.cron)#' $@	
	@echo "Crontab $@ created."

clean:
	@$(RM) reboot_proccheck.cron
