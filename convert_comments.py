#!/usr/bin/env python3

import re
import sys

def convert_block_comments(text):
    """Convert /* */ block comments to // line comments, preserving Doxygen format"""

    # Pattern to match block comments
    # This is complex because we need to handle multi-line comments
    pattern = r'/\*+([\s\S]*?)\*+/'

    def replace_comment(match):
        comment_content = match.group(1)

        # Check if this is a Doxygen comment (starts with /** or /*!)
        if comment_content.strip().startswith('*') or match.group(0).startswith('/**'):
            # This is already Doxygen style, convert to proper Doxygen
            lines = comment_content.split('\n')
            result_lines = []

            for line in lines:
                line = line.strip()
                if line.startswith('*'):
                    line = line[1:].strip()
                if line.startswith('@'):
                    result_lines.append(' * ' + line)
                elif line:
                    result_lines.append(' * ' + line)
                else:
                    result_lines.append(' *')

            return '/**\n' + '\n'.join(result_lines) + '\n */'
        else:
            # Regular block comment, convert to // comments
            lines = comment_content.split('\n')
            result_lines = []

            for line in lines:
                line = line.strip()
                if line:
                    result_lines.append('// ' + line)
                else:
                    result_lines.append('//')

            return '\n'.join(result_lines)

    return re.sub(pattern, replace_comment, text)

def convert_doxygen_comments(text):
    """Convert old Doxygen style to new @brief style"""

    # Pattern for old Doxygen comments
    pattern = r'/\*\*+\s*\n\s*\*\s*([^\n]*)\n([\s\S]*?)\s*\*/'

    def replace_doxygen(match):
        first_line = match.group(1).strip()
        rest = match.group(2).strip()

        # Check if it already has @brief
        if '@brief' in first_line or rest.startswith('@brief'):
            return match.group(0)

        # Add @brief to the first meaningful line
        if first_line:
            result = '/**\n * @brief ' + first_line
        else:
            # Find the first non-empty line in rest
            lines = rest.split('\n')
            first_content_line = None
            remaining_lines = []

            for i, line in enumerate(lines):
                line = line.strip()
                if line.startswith('*'):
                    line = line[1:].strip()
                if line and not line.startswith('@') and first_content_line is None:
                    first_content_line = line
                    remaining_lines = lines[i+1:]
                    break
                remaining_lines.append(line)

            if first_content_line:
                result = '/**\n * @brief ' + first_content_line
                if remaining_lines:
                    result += '\n' + '\n'.join([' * ' + line.strip() if line.strip() else ' *' for line in remaining_lines])
            else:
                result = match.group(0)

        result += '\n */'
        return result

    return re.sub(pattern, replace_doxygen, text)

def main():
    if len(sys.argv) != 2:
        print("Usage: python convert_comments.py <file>")
        sys.exit(1)

    filename = sys.argv[1]

    with open(filename, 'r') as f:
        content = f.read()

    # Convert block comments to line comments
    content = convert_block_comments(content)

    # Convert Doxygen comments to @brief style
    content = convert_doxygen_comments(content)

    with open(filename, 'w') as f:
        f.write(content)

    print(f"Converted comments in {filename}")

if __name__ == '__main__':
    main()
