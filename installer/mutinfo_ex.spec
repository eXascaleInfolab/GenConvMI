# -*- mode: python -*-
a = Analysis([os.path.join(HOMEPATH,'support/_mountzlib.py'), os.path.join(HOMEPATH,'support/useUnicode.py'), '../scripts/mutinfo_ex.py'],
             pathex=['/home/alcides/programming/projects/gecmi/installer'])
pyz = PYZ(a.pure)
exe = EXE( pyz,
          a.scripts,
          a.binaries,
          a.zipfiles,
          a.datas,
          name=os.path.join('dist', 'mutinfo_ex'),
          debug=False,
          strip=False,
          upx=True,
          console=1 )
