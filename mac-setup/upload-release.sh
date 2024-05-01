#!/usr/bin/env bash
# See https://cirrus-ci.org/examples/#release-assets

if [[ "$CIRRUS_RELEASE" == "" ]]; then
  echo "Not a release. No need to deploy!"
  exit 0
fi

if [[ "$GITHUB_TOKEN" == "" ]]; then
  echo "Please provide GitHub access token via GITHUB_TOKEN environment variable!"
  exit 1
fi

file_content_type="application/octet-stream"
asset_path=mac-setup/Xournal++.dmg  # relative path of asset to upload

ver=$(cat build/VERSION | sed '1q;d')  # --> x.y.z
name="xournalpp-$ver-macos_arm64.dmg"  # --> xournalpp-x.y.z-macos_arm64.dmg

echo "Uploading $asset_path..."
url_to_upload="https://uploads.github.com/repos/$CIRRUS_REPO_FULL_NAME/releases/$CIRRUS_RELEASE/assets?name=$name"
curl -X POST \
    --data-binary @$asset_path \
    --header "Authorization: token $GITHUB_TOKEN" \
    --header "Content-Type: $file_content_type" \
    $url_to_upload
