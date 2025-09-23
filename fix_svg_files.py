#!/usr/bin/env python3
"""
Simple SVG Processing Script
Fixes malformed SVG files and processes them to:
1. Make backgrounds transparent
2. Change all strokes and text to #6ceebe color
3. Remove duplicate attributes and fix XML structure
"""

import os
import re
from pathlib import Path

def fix_svg_file(file_path):
    """Fix a single SVG file by cleaning up malformed XML and applying color changes"""
    print(f"Processing: {file_path}")
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        
        # 1. Fix malformed XML by removing duplicate attributes
        # Remove duplicate stroke attributes
        content = re.sub(r'stroke="#6ceebe"[^>]*stroke="#6ceebe"', 'stroke="#6ceebe"', content)
        content = re.sub(r'stroke="#6ceebe"[^>]*stroke="#6ceebe"[^>]*stroke="#6ceebe"', 'stroke="#6ceebe"', content)
        
        # Remove duplicate fill attributes
        content = re.sub(r'fill="#6ceebe"[^>]*fill="#6ceebe"', 'fill="#6ceebe"', content)
        content = re.sub(r'fill="#6ceebe"[^>]*fill="#6ceebe"[^>]*fill="#6ceebe"', 'fill="#6ceebe"', content)
        
        # Fix malformed closing tags (remove extra attributes after />)
        content = re.sub(r'([^>]*?)/>([^>]*?)stroke="#6ceebe"', r'\1/>', content)
        content = re.sub(r'([^>]*?)/>([^>]*?)fill="#6ceebe"', r'\1/>', content)
        
        # 2. Make background transparent
        # Handle background rectangles
        content = re.sub(r'id="canvas_background"[^>]*fill="[^"]*"', 'id="canvas_background" fill="none"', content)
        content = re.sub(r'id="backgroundrect"[^>]*fill="[^"]*"', 'id="backgroundrect" fill="none"', content)
        
        # Replace common background colors with transparent
        background_colors = ['#f0f0f0', '#ff5555', '#ffffff', '#fff', '#000000']
        for color in background_colors:
            content = re.sub(f'fill="{color}"', 'fill="none"', content)
        
        # 3. Change all stroke colors to #6ceebe
        content = re.sub(r'stroke="#[^"]*"', 'stroke="#6ceebe"', content)
        content = re.sub(r'stroke="null"', 'stroke="#6ceebe"', content)
        
        # 4. Change fill colors to #6ceebe (but preserve transparent backgrounds)
        content = re.sub(r'fill="#[^"]*"', 'fill="#6ceebe"', content)
        
        # 5. Handle style attributes
        content = re.sub(r'stroke:#[^;"]*', 'stroke:#6ceebe', content)
        content = re.sub(r'fill:#[^;"]*', 'fill:#6ceebe', content)
        
        # 6. Ensure background elements stay transparent
        content = re.sub(r'id="canvas_background"[^>]*fill="#6ceebe"', 'id="canvas_background" fill="none"', content)
        content = re.sub(r'id="backgroundrect"[^>]*fill="#6ceebe"', 'id="backgroundrect" fill="none"', content)
        
        # 7. Add stroke to elements that don't have it (simple approach)
        # For path elements
        content = re.sub(r'<path([^>]*?)(?<!stroke=)[^>]*>', r'<path\1 stroke="#6ceebe">', content)
        
        # For polygon elements
        content = re.sub(r'<polygon([^>]*?)(?<!stroke=)[^>]*>', r'<polygon\1 stroke="#6ceebe">', content)
        
        # For line elements
        content = re.sub(r'<line([^>]*?)(?<!stroke=)[^>]*>', r'<line\1 stroke="#6ceebe">', content)
        
        # For rect elements
        content = re.sub(r'<rect([^>]*?)(?<!stroke=)[^>]*>', r'<rect\1 stroke="#6ceebe">', content)
        
        # For circle elements
        content = re.sub(r'<circle([^>]*?)(?<!stroke=)[^>]*>', r'<circle\1 stroke="#6ceebe">', content)
        
        # For ellipse elements
        content = re.sub(r'<ellipse([^>]*?)(?<!stroke=)[^>]*>', r'<ellipse\1 stroke="#6ceebe">', content)
        
        # 8. Add fill to text elements
        content = re.sub(r'<text([^>]*?)(?<!fill=)[^>]*>', r'<text\1 fill="#6ceebe">', content)
        content = re.sub(r'<tspan([^>]*?)(?<!fill=)[^>]*>', r'<tspan\1 fill="#6ceebe">', content)
        
        # 9. Clean up any remaining duplicate attributes
        content = re.sub(r'stroke="#6ceebe"[^>]*stroke="#6ceebe"', 'stroke="#6ceebe"', content)
        content = re.sub(r'fill="#6ceebe"[^>]*fill="#6ceebe"', 'fill="#6ceebe"', content)
        
        # Write the modified content back to the file
        if content != original_content:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"  ✓ Fixed and updated: {file_path}")
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
        fix_svg_file(svg_file)
    
    print("-" * 50)
    print("Processing complete!")

if __name__ == "__main__":
    main() 