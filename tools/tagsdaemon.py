
import sys
import os
import subprocess
import os.path

def do_tags():
    "ctags -R -f project_tags src/"


def main():
    project_dir = os.environ['PROJECT_DIR']
    tags_target = os.path.join( project_dir, 'src')
    
