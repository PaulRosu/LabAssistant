#!/usr/bin/env python3
"""
SVG XML-safe Processing Script
Processes all SVG files in the svg folder to:
1. Make backgrounds transparent (fill="none" for backgrounds)
2. Set all strokes and text fills to #6ceebe
3. Add missing stroke attributes where needed
4. Preserve valid SVG structure (no malformed XML)
"""

import os
import glob
from xml.etree import ElementTree as ET
from pathlib import Path

SVG_DIR = 'svg'
TARGET_COLOR = '#6ceebe'

# SVG elements that can have stroke
STROKE_TAGS = {'path', 'rect', 'circle', 'ellipse', 'line', 'polyline', 'polygon'}

# SVG elements that can have fill (text, shapes)
FILL_TAGS = {'text', 'tspan', 'rect', 'circle', 'ellipse', 'polygon', 'path', 'g'}

def process_svg(file_path):
    print(f'Processing: {file_path}')
    ET.register_namespace('', "http://www.w3.org/2000/svg")
    tree = ET.parse(file_path)
    root = tree.getroot()

    # 1. Make backgrounds transparent
    for elem in root.iter():
        # Common background rects: id or style or just first rect
        if elem.tag.endswith('rect') and (
            elem.attrib.get('id', '').lower() == 'canvas_background' or
            (elem.attrib.get('x') == '-1' and elem.attrib.get('y') == '-1') or
            (elem.attrib.get('fill') and elem.attrib['fill'] not in ('none', 'transparent'))
        ):
            elem.set('fill', 'none')

    # 2. Set all strokes to TARGET_COLOR
    for elem in root.iter():
        tag = elem.tag.split('}')[-1]
        if tag in STROKE_TAGS:
            elem.set('stroke', TARGET_COLOR)
        # Remove stroke-opacity if present (optional)
        if 'stroke-opacity' in elem.attrib:
            del elem.attrib['stroke-opacity']

    # 3. Set all text and shape fills to TARGET_COLOR
    for elem in root.iter():
        tag = elem.tag.split('}')[-1]
        if tag in FILL_TAGS:
            # Only set fill if not background
            if not (tag == 'rect' and elem.attrib.get('fill') == 'none'):
                elem.set('fill', TARGET_COLOR)
        # For <g> groups, do not override fill if it's a background
        if tag == 'g' and elem.attrib.get('id', '').lower() == 'canvas_background':
            elem.set('fill', 'none')

    # 4. Write back
    tree.write(file_path, encoding='utf-8', xml_declaration=True)

if __name__ == '__main__':
    svg_files = glob.glob(os.path.join(SVG_DIR, '*.svg'))
    for svg_file in svg_files:
        try:
            process_svg(svg_file)
        except Exception as e:
            print(f'Error processing {svg_file}: {e}') 