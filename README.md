# cmake_template

[![ci](https://github.com/fraserwat/cpp-template-redux/actions/workflows/ci.yml/badge.svg)](https://github.com/fraserwat/cpp-template-redux/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/fraserwat/cpp-template-redux/branch/main/graph/badge.svg)](https://codecov.io/gh/fraserwat/cpp-template-redux)
[![CodeQL](https://github.com/fraserwat/cpp-template-redux/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/fraserwat/cpp-template-redux/actions/workflows/codeql-analysis.yml)

## cmake_template_redux

This is a stripped down version of the C++ template it is forked from (https://github.com/cpp-best-practices/cmake_template).

Some changes:

 * Moved from Catch2 to GoogleTest. This was because I am already familiar with GoogleTest.
 * Warnings as errors, but only on code native to this project. There are some warnings which GoogleTest came with and imo this is too strict a setting. I am comfortable with the best practice that code I am producing should not be producing errors by default. Caveats around safe imports of trusted libraries still apply. 

## Getting Started

### Use the GitHub template
First, click the green `Use this template` button near the top of this page.
This will take you to GitHub's ['Generate Repository'](https://github.com/cpp-best-practices/cmake_template/generate)
page.
Fill in a repository name and short description, and click 'Create repository from template'.
This will allow you to create a new repository in your GitHub account,
prepopulated with the contents of this project.

After creating the project please wait until the cleanup workflow has finished 
setting up your project and committed the changes.

Now you can clone the project locally and get to work!

    git clone https://github.com/<user>/<your_new_repo>.git

