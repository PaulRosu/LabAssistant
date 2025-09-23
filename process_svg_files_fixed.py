#!/usr/bin/env python3
"""
Fixed SVG Processing Script
Processes all SVG files in the svg folder to:
1. Make backgrounds transparent
2. Change all strokes and text to #6ceebe color
3. Properly handle XML structure without creating malformed files
"""

import os
import re
import glob
from pathlib import Path

def process_svg_file_fixed(file_path):
    """Process a single SVG file with proper XML handling"""
    print(f"Processing: {file_path}")
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        
        # 1. Make background transparent
        # Replace common background colors with transparent
        background_patterns = [
            (r'fill="#[fF][0-9a-fA-F]{5}"', 'fill="none"'),  # #f0f0f0, #ff5555, etc.
            (r'fill="white"', 'fill="none"'),
            (r'fill="black"', 'fill="none"'),
        ]
        
        for pattern, replacement in background_patterns:
            content = re.sub(pattern, replacement, content)
        
        # 2. Change stroke colors to #6ceebe
        # Replace existing stroke colors
        stroke_patterns = [
            (r'stroke="#[0-9a-fA-F]{6}"', 'stroke="#6ceebe"'),
            (r'stroke="black"', 'stroke="#6ceebe"'),
            (r'stroke="white"', 'stroke="#6ceebe"'),
        ]
        
        for pattern, replacement in stroke_patterns:
            content = re.sub(pattern, replacement, content)
        
        # 3. Add stroke="#6ceebe" to elements that don't have stroke attribute
        # Only add to elements that don't already have a stroke attribute
        elements_to_process = ['path', 'polygon', 'line', 'rect', 'circle', 'ellipse']
        
        for element in elements_to_process:
            # Pattern to match elements without stroke attribute
            pattern = rf'(<{element}[^>]*?)(?<!stroke="[^"]*")(\s+[^>]*?>)'
            
            def add_stroke(match):
                tag_start = match.group(1)
                tag_end = match.group(2)
                # Only add stroke if it doesn't already exist
                if 'stroke=' not in tag_start and 'stroke=' not in tag_end:
                    return f'{tag_start} stroke="#6ceebe"{tag_end}'
                return match.group(0)
            
            content = re.sub(pattern, add_stroke, content, flags=re.DOTALL)
        
        # 4. Change fill colors to #6ceebe (except for background elements)
        # Replace existing fill colors, but avoid background rectangles
        fill_patterns = [
            (r'fill="#[0-9a-fA-F]{6}"', 'fill="#6ceebe"'),
            (r'fill="black"', 'fill="#6ceebe"'),
            (r'fill="white"', 'fill="#6ceebe"'),
        ]
        
        for pattern, replacement in fill_patterns:
            content = re.sub(pattern, replacement, content)
        
        # 5. Handle style attributes
        # Change colors in style attributes
        style_patterns = [
            (r'fill:\s*#[0-9a-fA-F]{6}', 'fill: #6ceebe'),
            (r'stroke:\s*#[0-9a-fA-F]{6}', 'stroke: #6ceebe'),
            (r'fill:\s*black', 'fill: #6ceebe'),
            (r'stroke:\s*black', 'stroke: #6ceebe'),
        ]
        
        for pattern, replacement in style_patterns:
            content = re.sub(pattern, replacement, content)
        
        # 6. Handle text elements specifically
        # Add fill="#6ceebe" to text elements that don't have it
        text_pattern = r'(<text[^>]*?)(?<!fill="[^"]*")(\s+[^>]*?>)'
        
        def add_text_fill(match):
            tag_start = match.group(1)
            tag_end = match.group(2)
            if 'fill=' not in tag_start and 'fill=' not in tag_end:
                return f'{tag_start} fill="#6ceebe"{tag_end}'
            return match.group(0)
        
        content = re.sub(text_pattern, add_text_fill, content, flags=re.DOTALL)
        
        # Only write if content changed
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
    
    svg_files = list(svg_folder.glob("*.svg"))
    
    if not svg_files:
        print(f"No SVG files found in {svg_folder}")
        return
    
    print(f"Found {len(svg_files)} SVG files to process...")
    print("=" * 50)
    
    for svg_file in svg_files:
        process_svg_file_fixed(svg_file)
    
    print("=" * 50)
    print("Processing complete!")

if __name__ == "__main__":
    main() 