## Test Ubuntu PPA Build
##
## Preparation:
## sudo apt install git-build-recipe
##
## PPA Config URL: https://code.launchpad.net/~andreasbutti/+recipe/xournalpp-daily

git-build-recipe --package xournalpp --allow-fallback-to-native xournalpp-ubuntu-ppa.recipe ppa-build




