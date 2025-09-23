#!/usr/bin/env python3
"""
Comprehensive SVG Processing Script
Processes all SVG files in the svg folder to:
1. Make backgrounds transparent
2. Change all strokes and text to #6ceebe color
3. Handle all edge cases including default colors and missing attributes
"""

import os
import re
import glob
from pathlib import Path

def process_svg_file_comprehensive(file_path):
    """Process a single SVG file with comprehensive color replacement"""
    print(f"Processing: {file_path}")
    
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        
        # 1. Make background transparent
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
        
        # Handle background rectangles specifically
        content = re.sub(r'id="canvas_background"[^>]*fill="[^"]*"', 'id="canvas_background" fill="none"', content)
        content = re.sub(r'id="backgroundrect"[^>]*fill="[^"]*"', 'id="backgroundrect" fill="none"', content)
        
        # 2. Add stroke="#6ceebe" to path elements that don't have stroke attributes
        # This handles the case where paths use default black stroke
        content = re.sub(r'<path([^>]*?)(?:\s+stroke="[^"]*")?([^>]*?)>', r'<path\1\2 stroke="#6ceebe">', content)
        
        # 3. Add stroke="#6ceebe" to polygon elements that don't have stroke attributes
        content = re.sub(r'<polygon([^>]*?)(?:\s+stroke="[^"]*")?([^>]*?)>', r'<polygon\1\2 stroke="#6ceebe">', content)
        
        # 4. Add stroke="#6ceebe" to line elements that don't have stroke attributes
        content = re.sub(r'<line([^>]*?)(?:\s+stroke="[^"]*")?([^>]*?)>', r'<line\1\2 stroke="#6ceebe">', content)
        
        # 5. Add stroke="#6ceebe" to rect elements that don't have stroke attributes
        content = re.sub(r'<rect([^>]*?)(?:\s+stroke="[^"]*")?([^>]*?)>', r'<rect\1\2 stroke="#6ceebe">', content)
        
        # 6. Add stroke="#6ceebe" to circle elements that don't have stroke attributes
        content = re.sub(r'<circle([^>]*?)(?:\s+stroke="[^"]*")?([^>]*?)>', r'<circle\1\2 stroke="#6ceebe">', content)
        
        # 7. Add stroke="#6ceebe" to ellipse elements that don't have stroke attributes
        content = re.sub(r'<ellipse([^>]*?)(?:\s+stroke="[^"]*")?([^>]*?)>', r'<ellipse\1\2 stroke="#6ceebe">', content)
        
        # 8. Change existing stroke colors to #6ceebe
        content = re.sub(r'stroke="#[^"]*"', 'stroke="#6ceebe"', content)
        content = re.sub(r'stroke="null"', 'stroke="#6ceebe"', content)
        
        # 9. Handle stroke in style attributes
        content = re.sub(r'stroke:#[^;"]*', 'stroke:#6ceebe', content)
        
        # 10. Change fill colors to #6ceebe (but preserve transparent backgrounds)
        content = re.sub(r'fill="#[^"]*"', 'fill="#6ceebe"', content)
        content = re.sub(r'fill:#[^;"]*', 'fill:#6ceebe', content)
        
        # 11. Handle text elements specifically
        # Add fill="#6ceebe" to text elements that don't have fill attributes
        content = re.sub(r'<text([^>]*?)(?:\s+fill="[^"]*")?([^>]*?)>', r'<text\1\2 fill="#6ceebe">', content)
        content = re.sub(r'<tspan([^>]*?)(?:\s+fill="[^"]*")?([^>]*?)>', r'<tspan\1\2 fill="#6ceebe">', content)
        
        # Update text style attributes
        content = re.sub(r'style="[^"]*fill:#[^;"]*', 'style="fill:#6ceebe', content)
        content = re.sub(r'style="[^"]*fill:#[^;"]*;', 'style="fill:#6ceebe;', content)
        
        # 12. Handle any remaining color references
        content = re.sub(r'color:#[^;"]*', 'color:#6ceebe', content)
        
        # 13. Ensure background elements stay transparent
        content = re.sub(r'id="canvas_background"[^>]*fill="#6ceebe"', 'id="canvas_background" fill="none"', content)
        content = re.sub(r'id="backgroundrect"[^>]*fill="#6ceebe"', 'id="backgroundrect" fill="none"', content)
        
        # 14. Handle elements with style attributes that need stroke
        # Add stroke to elements with style but no stroke
        content = re.sub(r'style="([^"]*)"([^>]*?)(?:\s+stroke="[^"]*")?([^>]*?)>', 
                        lambda m: f'style="{m.group(1)};stroke:#6ceebe"{m.group(2)}{m.group(3)}>', content)
        
        # Write the modified content back to the file
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
        process_svg_file_comprehensive(svg_file)
    
    print("-" * 50)
    print("Processing complete!")

if __name__ == "__main__":
    main() 