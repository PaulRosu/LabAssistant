#!/usr/bin/env python3
"""
SVG Processing Script
Processes all SVG files in the svg folder to:
1. Make backgrounds transparent
2. Change all strokes and text to #6ceebe color
"""

import os
import re
import glob
from pathlib import Path

def process_svg_file(file_path):
    """Process a single SVG file to make background transparent and change colors to #6ceebe"""
    print(f"Processing: {file_path}")
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        
        # 1. Make background transparent
        # Replace background rectangles with fill="none"
        content = re.sub(r'fill="#[^"]*"', 'fill="none"', content)
        
        # 2. Change all stroke colors to #6ceebe
        content = re.sub(r'stroke="#[^"]*"', 'stroke="#6ceebe"', content)
        
        # 3. Change fill colors to #6ceebe (but not for background elements)
        # This is more complex - we need to avoid changing background fills
        content = re.sub(r'fill="(?!none)[^"]*"', 'fill="#6ceebe"', content)
        
        # 4. Handle style attributes that contain color information
        # Change stroke colors in style attributes
        content = re.sub(r'stroke:#[^;]*', 'stroke:#6ceebe', content)
        
        # Change fill colors in style attributes (but not for background)
        content = re.sub(r'fill:#(?!none)[^;]*', 'fill:#6ceebe', content)
        
        # 5. Handle text elements - add fill color to text styles
        # Find text elements and add fill color if not present
        text_pattern = r'(<text[^>]*style="[^"]*)"'
        def add_fill_to_text(match):
            style = match.group(1)
            if 'fill:' not in style:
                style += ';fill:#6ceebe'
            return style + '"'
        
        content = re.sub(text_pattern, add_fill_to_text, content)
        
        # 6. Handle tspan elements similarly
        tspan_pattern = r'(<tspan[^>]*style="[^"]*)"'
        def add_fill_to_tspan(match):
            style = match.group(1)
            if 'fill:' not in style:
                style += ';fill:#6ceebe'
            return style + '"'
        
        content = re.sub(tspan_pattern, add_fill_to_tspan, content)
        
        # Write the modified content back to the file
        if content != original_content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"  ✓ Updated {file_path}")
        else:
            print(f"  - No changes needed for {file_path}")
            
    except Exception as e:
        print(f"  ✗ Error processing {file_path}: {e}")

def main():
    """Main function to process all SVG files"""
    svg_folder = Path("svg")
    
    if not svg_folder.exists():
        print("Error: 'svg' folder not found!")
        return
    
    # Find all SVG files
    svg_files = list(svg_folder.glob("*.svg"))
    
    if not svg_files:
        print("No SVG files found in the svg folder!")
        return
    
    print(f"Found {len(svg_files)} SVG files to process:")
    for svg_file in svg_files:
        print(f"  - {svg_file.name}")
    
    print("\nProcessing files...")
    
    # Process each SVG file
    for svg_file in svg_files:
        process_svg_file(svg_file)
    
    print("\nProcessing complete!")

if __name__ == "__main__":
    main() 