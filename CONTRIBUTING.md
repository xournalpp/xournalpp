# Contributing to Xournal++

Thanks for your interest in contributing to Xournal++! The project is run
completely by volunteers in their spare time, so any contribution--no matter how
small--is greatly appreciated. In this file, we'll outline how you can
contribute to the project.

If you have questions after reading this file, feel free to ask them on our [Gitter channel](https://gitter.im/xournalpp/xournalpp).

## Overview

Several of the main ways that you can contribute to the project include:

* Updating the [website](https://github.com/xournalpp/xournalpp.github.io)
* [Contributing translation improvements](#contributing-translation-improvements)
* [Contributing code improvements](#contributing-code-improvements)

## Contributing website improvements

For making changes to the website, please go to [xournalpp/xournalpp.github.io](https://github.com/xournalpp/xournalpp.github.io).

## Contributing translation improvements

If you would like to contribute translations, you can submit improvements to
[our project on Crowdin](https://crowdin.com/project/xournalpp). The Crowdin
translations are merged back into the main code periodically, after which they
become available available in the nightly builds.

## Contributing code improvements

Xournal++ development primarily occurs on GitHub at the [xournalpp/xournalpp
repository](https://github.com/xournalpp/xournalpp). As a contributor, you
probably have a particular bug or feature that you are interested in working on.
Before you start, you should first look in the [issue
tracker](https://github.com/xournalpp/xournalpp/issues) to see if anyone has
reported your bug or has a similar idea for a feature. If not, you should first
submit a new issue detailing what you are about to do. This will allow you to
get feedback and ensure that you do not end up duplicating work. You can obtain
additional help by contacting community members through one of our [official
communication channels](https://xournalpp.github.io/community/help).

The process for contributing code changes works as follows:

1. Fork the main xournalpp repository and create commits on your own feature
   branch.
2. Submit a pull request (PR).
3. Wait for maintainers to review the PR and address the relevant feedback.
4. After receiving maintainer approval, the PR is merged after a short grace
   period.

### Creating a fork

To make a code change, you'll need to first fork the repository on GitHub, if
you haven't already. To do so, click the "Fork" button in the top right corner
of the repository's web page.

We assume that you are already familiar with Git, which we use to track changes.
An excellent resource for Git is the _Pro Git_ book by Scott Chacon and Ben
Straub, which is available online for free
[here](https://git-scm.com/book/en/v2).

You should try to first clone your fork and then compile it manually. The
instructions for compiling Xournal++ depend on your operating system:

* [`LinuxBuild.md`](readme/LinuxBuild.md)
* [`MacBuild.md`](readme/MacBuild.md),
* [`WindowsBuild.md`](readme/WindowsBuild.md)

### Code conventions and guidelines

Here are some of the _guidelines_ that we follow when working on the Xournal++
codebase. We emphasize _guidelines_ because these are ideal points that we'd
like to see in contributions, but we understand that perfectly following all
points is very difficult in practice.

General contribution guidelines:

* **The code is best developed on Linux, followed by Windows, followed by
  MacOS**. Most contributors are using some Linux distribution, so the support
  and tooling on Linux is the most mature. Windows is also supported as a
  development environment via MSYS2, but the setup is less documented and
  potentially more complicated than on Linux. The project is set up to _build_
  on MacOS, but there is no documentation regarding development on there.
  Contributions to the tooling, development setup, and documentation are
  welcome!
* **Please accompany your code changes with documentation**. Because many
  contributors come and go, it is important to communicate the context of your
  contributions (e.g., issues being solved, why designed in a particular way,
  etc.) to future contributors. Documentation includes in-code doc comments, git
  commit messages, and/or detailed PR descriptions.
* Pull requests should be broken up into a series of small, "atomic" commits.
  Larger contributions should be broken up into multiple pull requests. This
  should be done to the extent that it makes it easier for maintainers to review
  contributions and keep track of changes.
  * By "atomic" commits, we mean self-contained commits that compile
    independently. For example, if a commit fixes a bug and _also_ incidentally
    refactors the surrounding logic without changing behavior, it should first
    be broken up into a refactor followed by a bugfix. If the refactor is
    _necessary_ for the bugfix, then the necessary parts should be kept together
    in the same commit, and the other parts should be extracted into a separate
    commit (but kept in the same PR).
  * Pull requests should target a single concern, e.g. "Fix text rendering bug"
    or "Implement PDF text selection". If a PR involves several large, unrelated
    changes, then it should probably be broken up.
  * Note that these are guidelines--there is no hard or fast rule that
    determines how commits or pull requests should be broken up.

Mechanical issues:

* Xournal++ is a mixture of C++ and C code. This is unavoidable because we rely on
  many C libraries (e.g. GTK). Prefer C++ when possible, however.
* Although we do not have a strong opinion on code style, we use `clang-format`
  to enforce a _consistent_ code style. Feel free to write your code in whatever
  style you prefer, as long as you run `clang-format` to format your code
  afterwards.
  * The code style will be automatically checked by the CI system. PRs that are
    not formatted correctly will fail to build; to remedy this, the "Clang
    Format Applied" check will also provide a patch that can be applied to fix
    the formatting.
  * This reduces the number of changed lines due to different code styles.
    Therefore we have fewer merge conflicts and the code is also easier to read
    and review.
* **Optimize your code for clarity and readability, but do not be overly
  verbose**.
  * Use meaningful variable names whenever possible, e.g. use `centerX = (posRect.x2 - posRect.x1) / 2` instead of `a = (r.x2 - r.x1) / 2`.
  * However, if it is clear from the context what the meaning of a variable is,
    prefer using a short name, e.g. `cx = (pos.x2 - pos.x1) / 2`.
  * If the type of a variable in a variable declaration is clear from the
    context, use `auto`:
    `auto foo = createFoo();`
* Follow modern C++ best practices as listed in the [C++ Core
  Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines).
  * In particular, make variables `const` by default, always use smart pointers
    instead of `new`/`delete`, and prefer `std::optional` over null pointers.
  * A useful tool to help you to fulfill those rules is `clang-tidy`. It can be
    found on almost all distro package managers.
* When working with code that interfaces with C libraries (GLib and GTK in
  particular), you should be extra careful with reference counting and memory
  management.
  * If you see a frequently occurring ref counting pattern, it might be helpful
    to create your own memory managing RAII ref count wrapper. This will also
    reduce the time others must spend to verify your code because it is correct
    by design.

See [the corresponding Wiki article](https://github.com/xournalpp/xournalpp/wiki/Coding-conventions) for more details on coding conventions.

### Sending your contributions for review

When you feel like you are ready to submit your code changes to be integrated
into the main Xournal++ repository or require some feedback on your work, you
will want to submit a pull request (PR). The steps to do so are outlined below.

1. Push your changes on to a branch of your fork in GitHub.
2. Hit the "New pull request" button on the
   [main repository's PR tracker](https://github.com/xournalpp/xournalpp/pulls).
   Select the branch you want to contribute, and then press "Create pull
   request." If you feel like your work isn't quite ready, feel free to submit
   your PR as a draft.
3. Write a short description of what your changes are and why you are making
   them. Please include any relevant issue numbers that your PR will fix, as
   well as a short description of what your changes are and why you are making
   them. Feel free to also include questions or comments about code snippets you
   may be unsure about. For examples of PR descriptions, take a look at some of
   the past PRs listed in the PR tracker.

### The review process

To maintain the quality of contributions, we typically require a PR to meet the
following requirements before it is merged.

* **The PR is reviewed by at least one maintainer**, and the relevant feedback is
  addressed.
* The code **successfully compiles** and passes all tests on our automated testing
  infrastructure.
* The code **passes the automated formatting check**. If the code does not pass,
  you can copy and paste the required changes from the Azure Pipelines page that
  has details on how the check failed.
* Before the PR is merged, a grace period of at least 24 hours has passed since
  the last maintainer has approved the PR. This allows time for objections or
  last-minute feedback. Larger or more complicated PRs may take a longer grace
  period.

Once a PR is ready to merge, we typically ask submitters to rebase or squash
their commits in order to keep the change history cleaner. If you do not know
how to do this, feel free to ask for help.
