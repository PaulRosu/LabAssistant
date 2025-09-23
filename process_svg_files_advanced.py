#!/usr/bin/env python3
"""
Advanced SVG Processing Script
Processes all SVG files in the svg folder to:
1. Make backgrounds transparent
2. Change all strokes and text to #6ceebe color
3. Handle edge cases and complex SVG structures
"""

import os
import re
import glob
from pathlib import Path
from xml.etree import ElementTree as ET

def process_svg_file_advanced(file_path):
    """Process a single SVG file using XML parsing for more accurate results"""
    print(f"Processing: {file_path}")
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        
        # 1. Make background transparent - handle various background patterns
        # Replace common background colors with transparent
        background_patterns = [
            (r'fill="#f0f0f0"', 'fill="none"'),
            (r'fill="#ff5555"', 'fill="none"'),
            (r'fill="#ffffff"', 'fill="none"'),
            (r'fill="#fff"', 'fill="none"'),
            (r'fill="#000000"', 'fill="none"'),  # For background elements
        ]
        
        for pattern, replacement in background_patterns:
            content = re.sub(pattern, replacement, content)
        
        # 2. Handle background rectangles specifically
        content = re.sub(r'id="canvas_background"[^>]*fill="[^"]*"', 'id="canvas_background" fill="none"', content)
        content = re.sub(r'id="backgroundrect"[^>]*fill="[^"]*"', 'id="backgroundrect" fill="none"', content)
        
        # 3. Change all stroke colors to #6ceebe
        # Handle stroke attributes
        content = re.sub(r'stroke="#[^"]*"', 'stroke="#6ceebe"', content)
        content = re.sub(r'stroke="null"', 'stroke="#6ceebe"', content)
        
        # Handle stroke in style attributes
        content = re.sub(r'stroke:#[^;"]*', 'stroke:#6ceebe', content)
        
        # 4. Change fill colors to #6ceebe (but preserve transparent backgrounds)
        # Only change fill colors that are not background-related
        content = re.sub(r'fill="#[^"]*"', 'fill="#6ceebe"', content)
        content = re.sub(r'fill:#[^;"]*', 'fill:#6ceebe', content)
        
        # 5. Handle text elements specifically
        # Find text elements and update their fill colors
        content = re.sub(r'style="[^"]*fill:#[^;"]*', 'style="fill:#6ceebe', content)
        content = re.sub(r'style="[^"]*fill:#[^;"]*;', 'style="fill:#6ceebe;', content)
        
        # 6. Handle path elements with stroke
        content = re.sub(r'<path[^>]*stroke="[^"]*"', lambda m: re.sub(r'stroke="[^"]*"', 'stroke="#6ceebe"', m.group(0)), content)
        
        # 7. Handle polygon elements with stroke
        content = re.sub(r'<polygon[^>]*stroke="[^"]*"', lambda m: re.sub(r'stroke="[^"]*"', 'stroke="#6ceebe"', m.group(0)), content)
        
        # 8. Handle any remaining color references
        content = re.sub(r'color:#[^;"]*', 'color:#6ceebe', content)
        
        # 9. Ensure background elements stay transparent
        # This is a safety check to make sure we don't accidentally color background elements
        content = re.sub(r'id="canvas_background"[^>]*fill="#6ceebe"', 'id="canvas_background" fill="none"', content)
        content = re.sub(r'id="backgroundrect"[^>]*fill="#6ceebe"', 'id="backgroundrect" fill="none"', content)
        
        # Write the modified content back to the file
        if content != original_content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"  ✓ Updated: {file_path}")
        else:
            print(f"  - No changes needed: {file_path}")
            
    except Exception as e:
        print(f"  ✗ Error processing {file_path}: {e}")

def process_svg_file_simple(file_path):
    """Simple regex-based processing for basic SVG files"""
    print(f"Processing: {file_path}")
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        
        # Make backgrounds transparent
        content = re.sub(r'fill="#f0f0f0"', 'fill="none"', content)
        content = re.sub(r'fill="#ff5555"', 'fill="none"', content)
        content = re.sub(r'fill="#ffffff"', 'fill="none"', content)
        content = re.sub(r'fill="#fff"', 'fill="none"', content)
        
        # Change strokes to #6ceebe
        content = re.sub(r'stroke="#[^"]*"', 'stroke="#6ceebe"', content)
        content = re.sub(r'stroke="null"', 'stroke="#6ceebe"', content)
        
        # Change fills to #6ceebe (but keep backgrounds transparent)
        content = re.sub(r'fill="#[^"]*"', 'fill="#6ceebe"', content)
        
        # Handle style attributes
        content = re.sub(r'fill:#[^;"]*', 'fill:#6ceebe', content)
        content = re.sub(r'stroke:#[^;"]*', 'stroke:#6ceebe', content)
        
        # Ensure backgrounds stay transparent
        content = re.sub(r'id="canvas_background"[^>]*fill="#6ceebe"', 'id="canvas_background" fill="none"', content)
        content = re.sub(r'id="backgroundrect"[^>]*fill="#6ceebe"', 'id="backgroundrect" fill="none"', content)
        
        if content != original_content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"  ✓ Updated: {file_path}")
        else:
            print(f"  - No changes needed: {file_path}")
            
    except Exception as e:
        print(f"  ✗ Error processing {file_path}: {e}")

def main():
    """Main function to process all SVG files"""
    svg_folder = Path("svg")
    
    if not svg_folder.exists():
        print(f"Error: {svg_folder} folder not found!")
        return
    
    # Find all SVG files
    svg_files = list(svg_folder.glob("*.svg"))
    
    if not svg_files:
        print("No SVG files found in the svg folder!")
        return
    
    print(f"Found {len(svg_files)} SVG files to process:")
    print("-" * 50)
    
    # Process each SVG file
    for svg_file in svg_files:
        # Use simple processing for most files, advanced for complex ones
        if svg_file.name in ['mouse.svg', 'mouse2.svg', 'OpM_rename.svg']:
            process_svg_file_advanced(svg_file)
        else:
            process_svg_file_simple(svg_file)
    
    print("-" * 50)
    print("Processing complete!")

if __name__ == "__main__":
    main() 