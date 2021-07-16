#  Log Documentation

## Logging Functions

### init_logging
**Synopsis**

Initialize logging for PennOS

void init_logging(char \*log_name);

**Description**

Creates a directory for logging if none exist by the name "log" yet. If a logfile of *log_name* has not yet been created, create a file that can be written onto.

**Return Value**

This function is a void function, so nothing is returned.

### log_event
**Synopsis**

Log an Event onto the Logfile

void log_event(unsigned long ticks, char \*func, pid_t pid, int nice, char \*cmd);

**Description**

Print critical details about the event onto the logfile:
* Tick count (time)
* Invoked Function Name
* PID
* Nice Value
* Process Name

**Return Value**

This function is a void function, so nothing is returned.

### logging_logout
**Synopsis**

Handles Logger logout

void logging_logout();

**Description**

Close the logfile prior to logging out and quitting.

**Return Value**

This function is a void function, so nothing is returned.
