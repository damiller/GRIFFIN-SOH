odbedit -c "set /Equipment/VME-01/Settings/mainSwitch 0"
odbedit -c "set /Equipment/VME-02/Settings/mainSwitch 0"
odbedit -c "set /Equipment/VME-03/Settings/mainSwitch 0"
odbedit -c "set /Equipment/VME-04/Settings/mainSwitch 0"
odbedit -c "set /Equipment/VME-05/Settings/mainSwitch 0"
odbedit -c "set /Equipment/VME-06/Settings/mainSwitch 0"
odbedit -c "set /Equipment/VME-07/Settings/mainSwitch 0"


odbedit -c "set /Equipment/VME-01/Settings/EnableControl 1"
odbedit -c "set /Equipment/VME-02/Settings/EnableControl 1"
odbedit -c "set /Equipment/VME-03/Settings/EnableControl 1"
odbedit -c "set /Equipment/VME-04/Settings/EnableControl 1"
odbedit -c "set /Equipment/VME-05/Settings/EnableControl 1"
odbedit -c "set /Equipment/VME-06/Settings/EnableControl 1"
odbedit -c "set /Equipment/VME-07/Settings/EnableControl 1"

sleep 10

odbedit -c "set /Equipment/VME-01/Settings/EnableControl 0"
odbedit -c "set /Equipment/VME-02/Settings/EnableControl 0"
odbedit -c "set /Equipment/VME-03/Settings/EnableControl 0"
odbedit -c "set /Equipment/VME-04/Settings/EnableControl 0"
odbedit -c "set /Equipment/VME-05/Settings/EnableControl 0"
odbedit -c "set /Equipment/VME-06/Settings/EnableControl 0"
odbedit -c "set /Equipment/VME-07/Settings/EnableControl 0"
