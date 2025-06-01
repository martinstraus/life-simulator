#!/bin/bash
# filepath: /home/martinstraus/proyectos/life-simulator/create_version.sh

# Usage: ./create_version.sh <version> "<description>"

set -e

if [ $# -lt 2 ]; then
    echo "Usage: $0 <version> \"<description>\""
    exit 1
fi

VERSION="$1"
DESCRIPTION="$2"
CHANGELOG="changelog.md"

# Add description to changelog
echo -e "## Version $VERSION\n$DESCRIPTION\n" >> "$CHANGELOG"

# Add and commit changelog
git add "$CHANGELOG"
git commit -m "Update changelog for version $VERSION"

# Create annotated tag
git tag -a "$VERSION" -m "$DESCRIPTION"

# Push commit and tag
git push
git push origin "$VERSION"

echo "Version $VERSION created and pushed."