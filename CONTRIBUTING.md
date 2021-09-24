# Contributing to Xournal++

Thanks for your interest in contributing to Xournal++! The project is run
completely by volunteers in their spare time, so any contribution--no matter how
small--is greatly appreciated. In this file, we'll outline how you can
contribute to the project.

Xournal++ development primarily occurs on [GitHub at the xournalpp/xournalpp
repository](https://github.com/xournalpp/xournalpp). As a contributor, you
probably have a particular bug or feature that you are interested in working on.
Before you start, you should first look in the [issue
tracker](https://github.com/xournalpp/xournalpp/issues) to see if anyone has
reported your bug or has a similar idea for a feature. If not, you should first
submit a new issue detailing what you are about to do. This will allow you to
get feedback and ensure that you do not end up duplicating work. You can obtain
additional help by contacting community members through one of our [official
communication channels](https://xournalpp.github.io/community/help/).

## Contributing translations

If you would like to contribute translations, you can submit improvements to
[our project on Crowdin](https://crowdin.com/project/xournalpp).
Those are then merged from time to time.
So they wont appear directly after your contribution.

## Contributing code changes with pull requests (PRs)

To make a code change, you'll need to first fork the repository on GitHub, if
you haven't already. To do so, click the "Fork" button in the top right corner
of the repository's web page.

We assume that you are already familiar with Git, which we use to track changes.
An excellent resource for Git is the _Pro Git_ book by Scott Chacon and Ben
Straub, which is available online for free
[here](https://git-scm.com/book/en/v2).

You should try to first clone your fork and then compile it manually. The
instructions for compiling Xournal++ on your operating system can be found at
[`LinuxBuild.md`](readme/LinuxBuild.md), [`MacBuild.md`](readme/MacBuild.md),
and [`WindowsBuild.md`](readme/WindowsBuild.md). Once you are set up, you will
be ready to make code changes.

### Code conventions and guidelines

Here is a quick list of guidelines that we follow when working on the Xournal++
codebase.

* Xournal++ is a mixture of C++ and C code. This is unavoidable because we rely on
  many C libraries (e.g. GTK). Prefer C++ when possible, however.
* Although we do not have a strong opinion on code style, we use `clang-format`
  to enforce a _consistent_ code style. Feel free to write your code in whatever
  style you prefer, as long as you run `clang-format` to format your code
  afterwards.
* Please accompany your code changes with documentation.
* Follow modern C++ best practices as listed in the [C++ Core
  Guidelines](https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines). In
  particular, make variables `const` by default, always use smart pointers
  instead of new/delete, and prefer `std::optional` over null pointers.
* When working with code that interfaces with C libraries (GLib and GTK in
  particular), you should be extra careful with reference counting and memory
  management.
* TODO, etc.

### Sending your contributions for review

When you feel like you are ready to submit your code changes to be integrated
into the main Xournal++ repository or require some feedback on your work, you will want to submit a pull request (PR).
To do so, push your changes on to a branch of your fork in GitHub, and then hit
the "New pull request" button on the [main repository's PR
tracker](https://github.com/xournalpp/xournalpp/pulls). Select the branch you
want to contribute, and then press "Create pull request." If you feel like your
work isn't quite ready, feel free to submit your PR as a draft.

In your description, please include any relevant issue numbers that your PR will
fix, as well as a short description of what your changes are and why you are
making them. Feel free to also include questions or comments about code snippets
you may be unsure about.

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
