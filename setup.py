from distutils.core import setup

setup(name='Pybag',
      version='1.0.0',
      description='Python wrappers for DbgEng from Windbg',
      author='Doug S',
      url='https://github.com/dshikashio/Pybag',
      packages=['pybag', 'pybag.dbgeng'],
      package_data={'pybag' : ['dbgeng/tlb/*.tlb']}
      )