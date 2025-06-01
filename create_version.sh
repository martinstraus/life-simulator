#!/bin/bash

# Usage: ./create_version.sh <version>

set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <version>"
    exit 1
fi

VERSION="$1"
CHANGELOG="changelog.md"

# Get the last tag (if any)
LAST_TAG=$(git describe --tags --abbrev=0 2>/dev/null || echo "")

# Get log entries since last tag
if [ -z "$LAST_TAG" ]; then
    LOG=$(git log --pretty=format:"- %s" )
else
    LOG=$(git log "$LAST_TAG"..HEAD --pretty=format:"- %s")
fi

if [ -z "$LOG" ]; then
    echo "No new commits since last tag. Aborting."
    exit 1
fi

# Add log to the beginning of changelog
TMP_CHANGELOG=$(mktemp)
echo -e "## Version $VERSION\n$LOG\n" > "$TMP_CHANGELOG"
cat "$CHANGELOG" >> "$TMP_CHANGELOG" 2>/dev/null || true
mv "$TMP_CHANGELOG" "$CHANGELOG"

# Add and commit changelog
git add "$CHANGELOG"
git commit -m "Update changelog for version $VERSION"

# Create annotated tag with the log as the message
git tag -a "$VERSION" -m "Changelog:\n$LOG"

# Push commit and tag
git push
git push origin "$VERSION"

echo "Version $VERSION created and pushed."