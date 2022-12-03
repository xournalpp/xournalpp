# Releases

## Versioning

Every release of Xournal++ is tagged with a unique incrementing version. In the history of the project, two changes to the versioning scheme happened. Be careful when trying to automatically parse older versions.
The current versioning scheme adheres to the rules of [Debian upstream_version](https://www.debian.org/doc/debian-policy/ch-controlfields.html#s-f-version).

### Main Version

We follow the common versioning scheme of major, minor and patch versions in this order, separated by dots.

The version string therefore has the following format:

```
[0-9]+\.[0-9]+\.[0-9]+
```

We do NOT adhere to semantic versioning, as it is primarily meant for libraries and is more confusing to normal users than it is helpful.
To help you understand when we use what version increment, use these rules:

- A major version bump happens when we introduce a change with a very high impact to many users. For example a major redesign of the GUI or the introduction of a new file format as the default format.
- A minor version bump happens whenever we introduce new features
- A patch version bump happens whenever we introduce fixes to already implemented features. This may include minor changes to existing features as well.

Whenever a bump of the major version happens, minor and patch versions are reset to `0`. Equally the patch version is reset to `0` when the minor version is bumped.

### Version Suffix

The main version string may be followed by a suffix. This suffix is used to describe unreleased versions and hotfixes.

The suffix may only start with one of the two characters `+` or `~`. These characters hold special meaning:

- `+` denotes a version that is based on top of the main version (is newer)
- `~` denotes a version that predates the main version (is older)

The suffix may only contain alphanumerics, `+`, `~`, `-` and `.`.

We mainly use suffixes to specify development versions. Our main development branch (`master`) has a permanent suffix `+dev` and as main version the last released version of Xournal++. This specifies that the current commit of the main development branch is based on the last release of Xournal++, but may contain any number of changes.

When we start our release process, we branch off a release branch. At this point of time it is clear what changes we include in the release and we can use the above rules to determine the correct version. From this point on we will change the main version string to this new version and use the suffix `~dev`. This describes our intent of working towards this release.

## Branching Strategy

Our branching model knows five types of branches:

- The main development branch (`master`) used for merging feature branches and hosting the newest state of Xournal++.
- Release branches (`release-*`) exist for preparing a release. This type of branches is branched off from the main development branch and will from that point on only receive bug fixes for the current version of this branch. Only the patch level may be bumped on such branches. When the release is ready, it is tagged on this branch and then merged back to the main development branch.
- Hotfix branches (`hotfix-*`) originate from the tagged commit of a released version and facilitate the development of a hotfix. When the hotfix is ready for release, it will be tagged and merged back into the main development branch.
- Feature branches (`feature-*`) are commonly used for Pull Requests with new features. Such branches are always merged into the main development branch.
- Bugfix branches (`bugfix-*`) are used for Pull Requests with a bugfix for a maintained release or the current development version. Depending on their target they must be merged into the appropriate branch.

## Helper Script

To help with the task of versioning, we provide a helper script, that does most of the work for you. You can find the script at `scripts/release_helper.sh`.

### Preparing a new minor/major release
The starting point of a new minor/major release is always the main development branch. You may use `release_helper.sh prepare <version>` to prepare the release. `<version>` Shall be replaced with the appropriate version. Be aware that the patch version must be `0`.

The script will:

1. Create a release branch
2. Check the new branch out
3. Apply the supplied version to all relevant files suffixed by `~dev`.

Whatever version suffix you choose, will be appended by `~dev` to signify that this is not yet the release version but development progresses towards this release.

### Preparing a hotfix
The starting point of a hotfix release is the tagged commit of the release this hotfix is based upon. You must check out this release and then call `release_helper.sh prepare <version><+suffix>` where the version equals the currently checked out version and the chosen suffix. The suffix must start with a `+` as only hotfixes based on top of this version are allowed.

The script will:

1. Create a hotfix branch
2. Check the new branch out
3. Apply the supplied version to all relevant files suffixed by `~dev`.

### Publishing a release
Publishing a release can only happen on a release branch or a hotfix branch. It can be triggered by `release_helper.sh publish` and will initiate the process of publishing the release.

The script will:

1. Remove the `~dev` suffix (and leave any other suffix supplied by the user)
2. Update the changelogs to signify the release of this version
3. Tag the release
4. Proposal to merge the release back to the main development branch

During the merge it is imperative that the instructions of the script are followed stringently to not confuse our versioning scheme.
