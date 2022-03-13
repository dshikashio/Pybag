REM discovery doesn't work well due to:
REM need to split dbgeng instances into different processe
REM until its clear how to properly release all references to objects

REM python -m unittest discover

python -m tests.test_idebugclient
python -m tests.test_target TestBasic
REM python -m tests.test_target TestTargetCreate
python -m tests.test_target TestTargetAttach1
python -m tests.test_target TestTargetAttach2