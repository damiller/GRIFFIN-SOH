echo "No action is required.

This is an automated email issued once per day to confirm that the GRIFFIN State-Of-Health machine is operational.

The status page is grifsoh00.triumf.ca:8081" | mail -r "$USER@$HOSTNAME" -s "GRIFFIN State-Of-Health OK" equip@grsimail.triumf.ca
