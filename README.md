# cmake_template

[![ci](https://github.com/fraserwat/cpp-template-redux/actions/workflows/ci.yml/badge.svg)](https://github.com/fraserwat/cpp-template-redux/actions/workflows/ci.yml)
[![codecov](https://codecov.io/fraserwat/cpp-template-redux/branch/main/graph/badge.svg)](https://codecov.io/gh/fraserwat/cpp-template-redux)
[![CodeQL](https://github.com/cpp-best-practices/cmake_template/actions/workflows/codeql-analysis.yml/badge.svg)](https://github.com/cpp-best-practices/cmake_template/actions/workflows/codeql-analysis.yml)

# C++ Daily News Roundup

In 2024 I had a full social media detox while I was finishing my masters degree and got my screentime down to ~1 hour/day. A side-effect of this. I think this is a good thing to do (to the point where I think it is useful and healthy to just _not have your phone on you_ sometimes), but a side-effect was I felt like I was totally uninformed about anything that was going on "in the world". Some of this might have been said masters degree I was finishing off, and the guilt you feel when doing anything like this (part-time) when you're reading anything that isn't, I dunno, a paper on recommender systems. Regardless, I wanted to learn Rust anyway (the algorithm I was forking for my thesis was implemented in Rust) so I built a [daily news round-up](https://github.com/fraserwat/newsround) which was sent to my inbox.

Since then, there's a couple of things I would have changed, a couple of things that have since broken, and I am learning a new language (C++), so I have refactored it into C++. This is that.

## cmake_template_redux

This is a stripped down version of [the C++ template it is forked from](https://github.com/cpp-best-practices/cmake_template).

Some changes:

 * Moved from Catch2 to GoogleTest. This was because I am already familiar with GoogleTest.
 * Warnings as errors, but only on code native to this project. There are some warnings which GoogleTest came with and imo this is too strict a setting. I am comfortable with the best practice that code I am producing should not be producing errors by default. Caveats around safe imports of trusted libraries still apply. 

## Known Issues / TODO: Needs Proper Docs

### macOS: `malloc: nano zone abandoned due to inability to reserve vm space`

When running the app on macOS you may see:

```
compiled_app(...) malloc: nano zone abandoned due to inability to reserve vm space.
```

Things like the Address Sanitiser (used by curl in this app) crowd out the nano zone on macOS, so this particular malloc optimisation is going to raise a warning -- this is intentional! So will suppress.

The fix is to set `MallocNanoZone=0` in your environment before running the binary. CMake has no mechanism to inject environment variables into a directly-executed binary (only into CTest-managed processes), so this needs to be documented properly and handled at the shell/docs level.

## Prerequisites

The following system libraries must be installed before building. On macOS:

```
brew install curl rocksdb
```

- **curl** — HTTP requests for scraping
- **rocksdb** — persistent key-value store for deduplicating seen stories

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

