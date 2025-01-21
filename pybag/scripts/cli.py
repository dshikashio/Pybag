import argparse
import sys

import cmd2

class PybagCli(cmd2.Cmd):
    def __init__(self):
        super().__init__(self, include_py=True)
        self.varxxx = 1
        self.completekey = 'tab'

    def do_blah(self, args):
        print('help')


    def do_go(self, args):
        pass

    def do_goto(self, args):
        pass

    def do_stepi(self, args):
        pass

    def do_stepo(self, args):
        pass

    def do_stepbr(self, args):
        pass

    def do_stepto(self, args):
        pass

    def do_steopout(self, args):
        pass

    def do_trace(self, args):
        pass

    def do_traceto(self, args):
        pass
    
    def do_cmd(self, args):
        pass

    def do_windbgshell(self, args):
        pass

    def do_read(self, args):
        pass

    def do_write(self, args):
        pass

    def do_address(self, args):
        pass

    


def getargs(cmdline):
    pass

def cli(argv=sys.argv):
    args = getargs(argv)
    # if kernel
    # if user
    # if ...
    app = PybagCli()
    app.cmdloop()


if __name__ == '__main__':
    cli()