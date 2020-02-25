import unittest
import grf
import os
import pprint


class MyTestCase(unittest.TestCase):
    def setUp(self) -> None:
        self.filename = '/home/colin/.local/share/lutris/runners/winesteam/prefix64/drive_c/Program Files (x86)/Steam/steamapps/common/Ragnarok/data.grf'
        self.archive = grf.Archive(self.filename)

    def test_open(self):
        self.assertIsNotNone(self.archive)

    def test_nfiles(self):
        self.assertEqual(self.archive.nfiles, 78113)

    def test_filename(self):
        self.assertEqual(self.archive.filename, self.filename)

    def test_version(self):
        self.assertEqual(self.archive.version, 0x200)

    def test_files(self):
        files = self.archive.files
        for f in files:
            pass

    def test_extract(self):
        for i, f in enumerate(self.archive.files):
            filename = f.name.decode('cp949')
            filename = filename.replace('\\', '/')
            dir = os.path.dirname(filename)
            os.makedirs(dir, exist_ok=True)
            try:
                self.archive.extract(f.name, filename)
            except IOError as e:
                print(e)
                pass


if __name__ == '__main__':
    unittest.main()
