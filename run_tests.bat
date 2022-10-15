REM Use auto test discovery

python -m unittest discover


REM If discovery doesn't work well:
REM need to split dbgeng instances into different processes
REM until its clear how to properly release all references to objects

REM python -m tests.test_idebugclient
REM python -m tests.test_target TestBasic
REM python -m tests.test_target TestTargetCreate
REM python -m tests.test_target TestTargetAttach1
REM python -m tests.test_target TestTargetAttach2