import pathlib
from setuptools import setup

HERE = pathlib.Path(__file__).parent

README = (HERE / "README.md").read_text()

setup(name='Pybag',
      version='2.2.7',
      description='Python wrappers for DbgEng from Windbg',
      long_description=README,
      long_description_content_type="text/markdown",
      author='Doug S',
      url='https://github.com/dshikashio/Pybag',
      license="MIT",
      keywords=['windows', 'debugging', 'dbgeng', 'windbg', 'reverseengineering'],
      classifiers=[
          'License :: OSI Approved :: MIT License',
          'Programming Language :: Python :: 3',
          'Development Status :: 5 - Production/Stable',
          'Intended Audience :: Developers',
          'Intended Audience :: Science/Research',
          'Natural Language :: English',
          'Operating System :: Microsoft :: Windows',
          'Programming Language :: Python',
          'Topic :: Software Development :: Libraries :: Python Modules',
      ],
      packages=['pybag', 'pybag.dbgeng', 'pybag.dbgeng.win32'],
      package_data={'pybag': ['dbgeng/tlb/*.tlb']},
      include_package_data=True,
      install_requires=[
          'capstone>=4.0.2',
          'comtypes>=1.1.14',
          'pywin32>=301',
      ],
      python_requires=">=3.6.0",
)
