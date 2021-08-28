import pathlib
from setuptools import setup

HERE = pathlib.Path(__file__).parent

README = (HERE / "README.md").read_text()

setup(name='Pybag',
      version='2.2.1',
      description='Python wrappers for DbgEng from Windbg',
      long_description=README,
      long_description_content_type="text/markdown",
      author='Doug S',
      url='https://github.com/dshikashio/Pybag',
      license="MIT",
      classifiers=[
          "License :: OSI Approved :: MIT License",
          "Programming Language :: Python :: 3",
          "Programming Language :: Python :: 3.8",
      ],      
      packages=['pybag', 'pybag.dbgeng'],
      package_data={'pybag' : ['dbgeng/tlb/*.tlb']},
      include_package_data=True,
      install_requires=[
          'capstone>=4.0.2',
          'comtypes>=1.1.10',
          'pywin32>=301',
      ]
 )
