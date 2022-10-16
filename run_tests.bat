REM Use auto test discovery

REM python -m unittest discover


REM If discovery doesn't work well:
REM need to split dbgeng instances into different processes
REM until its clear how to properly release all references to objects

python -m tests.test_idebugclient
python -m tests.test_target TestBasic
python -m tests.test_target TestTargetCreate
python -m tests.test_target TestTargetAttach1
python -m tests.test_target TestTargetAttach2
python -m tests.test_target TestStringCmd
