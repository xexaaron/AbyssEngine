# main.py

import os
import re

def define_env(env):
    @env.macro
    def get_sections():
        base_path = os.path.join(env.project_dir, "docs", "pages")
        entries = []
        for name in os.listdir(base_path):
            entries.append(str.removesuffix(name, ".md"))
        return sorted(entries)
    
    @env.macro
    def get_icon(section):
        if section == "Object":
            return "material-cube-outline"
        if section == "Shaders":
            return "fontawesome-solid-cubes"
        if section == "Resource":
            return "octicons-image-24"
        if section == "Text":
            return "material-format-text"
        
    @env.macro
    def include_clean(path):
        with open(path, "r") as f:
            code = f.read()
        # Remove line comments
        code = re.sub(r'//.*', '', code)
        # Remove block comments
        code = re.sub(r'/\*.*?\*/', '', code, flags=re.DOTALL)
        return code.strip()