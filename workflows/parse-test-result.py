#!/usr/bin/env python3
"""
Test results parser for Git-RAF sinphase calculation
Supports multiple test formats: JUnit XML, TAP, simple text
"""

import xml.etree.ElementTree as ET
import re
import sys
import os

def parse_junit_xml(file_path):
    """Parse JUnit XML test results"""
    tree = ET.parse(file_path)
    root = tree.getroot()
    
    tests = int(root.attrib.get('tests', 0))
    failures = int(root.attrib.get('failures', 0))
    errors = int(root.attrib.get('errors', 0))
    
    passes = tests - failures - errors
    return passes, tests

def parse_tap_file(file_path):
    """Parse TAP (Test Anything Protocol) results"""
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Count test points
    test_points = re.findall(r'^(ok|not ok)\s+\d+', content, re.MULTILINE)
    tests = len(test_points)
    passes = len([tp for tp in test_points if tp.startswith('ok')])
    
    return passes, tests

def parse_simple_text(file_path):
    """Parse simple text test results"""
    with open(file_path, 'r') as f:
        content = f.read()
    
    # Look for common patterns
    patterns = [
        r'(\d+)\s+pass', r'(\d+)\s+passed',
        r'(\d+)\s+fail', r'(\d+)\s+failed',
        r'(\d+)\s+test', r'Tests:\s*(\d+)'
    ]
    
    passes = 0
    tests = 0
    
    for pattern in patterns:
        matches = re.findall(pattern, content, re.IGNORECASE)
        if matches:
            if 'pass' in pattern:
                passes = int(matches[0])
            elif 'test' in pattern:
                tests = int(matches[0])
    
    return passes, tests

def main():
    if len(sys.argv) < 2:
        print("Usage: parse-test-results.py <test-results-file>")
        sys.exit(1)
    
    file_path = sys.argv[1]
    
    if not os.path.exists(file_path):
        print(f"File not found: {file_path}")
        sys.exit(1)
    
    # Determine file type and parse accordingly
    if file_path.endswith('.xml'):
        passes, tests = parse_junit_xml(file_path)
    elif file_path.endswith('.tap'):
        passes, tests = parse_tap_file(file_path)
    else:
        passes, tests = parse_simple_text(file_path)
    
    print(f"{passes} {tests}")

if __name__ == '__main__':
    main()
