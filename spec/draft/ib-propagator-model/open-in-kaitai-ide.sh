#!/usr/bin/env bash
# Open Kaitai Struct file in Web IDE

set -euo pipefail

KSY_FILE="${1:-sprite-lfs.ksy}"
WEB_IDE_URL="https://ide.kaitai.io/"

if [ ! -f "$KSY_FILE" ]; then
    echo "Error: File '$KSY_FILE' not found"
    echo "Usage: $0 [file.ksy]"
    exit 1
fi

echo "📝 Opening Kaitai Struct Web IDE..."
echo "📄 File: $KSY_FILE"
echo ""
echo "🌐 Opening browser to: $WEB_IDE_URL"
echo ""
echo "Instructions:"
echo "  1. Click 'Load .ksy file' button"
echo "  2. Select: $KSY_FILE"
echo "  3. Optionally upload a binary sample file"
echo ""

# Try to open in browser (works on most Linux systems)
if command -v xdg-open &> /dev/null; then
    xdg-open "$WEB_IDE_URL"
elif command -v open &> /dev/null; then
    open "$WEB_IDE_URL"
else
    echo "Please open this URL manually: $WEB_IDE_URL"
fi

echo "✨ Done! The Web IDE should open in your browser."

