from btree_node import BTreeNode
from btree_file import DiskManager
import bisect


class BTree:
    def __init__(self, filename="btree.dat"):
        self.disk = DiskManager(filename)
        self.root_offset = None
        if os.path.getsize(filename) == 0:
            # create root node
            root = BTreeNode(is_leaf=True)
            self.root_offset = self.disk.allocate_page()
            self.disk.write_page(self.root_offset, root)
        else:
            self.root_offset = 0  # assume root at offset 0

    def search(self, offset, key):
        node = self.disk.read_page(offset)
        i = bisect.bisect_left(node.keys, key)

        if node.is_leaf:
            if i < len(node.keys) and node.keys[i] == key:
                return node.values[i]
            else:
                return None
        else:
            child_offset = node.children[i]
            return self.search(child_offset, key)

    def insert(self, key, value):
        root = self.disk.read_page(self.root_offset)
        if len(root.keys) == 2 * ORDER - 1:
            # root is full → split
            new_root = BTreeNode(is_leaf=False)
            new_root.children.append(self.root_offset)
            new_root_offset = self.disk.allocate_page()
            self.split_child(new_root, 0, self.root_offset)
            self.root_offset = new_root_offset
            self.disk.write_page(new_root_offset, new_root)
            self.insert_non_full(new_root, key, value)
        else:
            self.insert_non_full(root, key, value)
            self.disk.write_page(self.root_offset, root)

    def insert_non_full(self, node, key, value):
        i = len(node.keys) - 1
        if node.is_leaf:
            node.keys.insert(bisect.bisect_left(node.keys, key), key)
            node.values.insert(bisect.bisect_left(node.keys, key), value)
            node.num_keys += 1
        else:
            i = bisect.bisect_left(node.keys, key)
            child_offset = node.children[i]
            child = self.disk.read_page(child_offset)
            if len(child.keys) == 2 * ORDER - 1:
                self.split_child(node, i, child_offset)
                if key > node.keys[i]:
                    i += 1
                child_offset = node.children[i]
                child = self.disk.read_page(child_offset)
            self.insert_non_full(child, key, value)
            self.disk.write_page(child_offset, child)

    def split_child(self, parent, idx, child_offset):
        child = self.disk.read_page(child_offset)
        new_child = BTreeNode(is_leaf=child.is_leaf)
        new_offset = self.disk.allocate_page()

        mid = ORDER - 1
        # promote middle key to parent
        parent.keys.insert(idx, child.keys[mid])
        parent.children.insert(idx + 1, new_offset)

        # new right node
        new_child.keys = child.keys[mid + 1:]
        child.keys = child.keys[:mid]

        if child.is_leaf:
            new_child.values = child.values[mid + 1:]
            child.values = child.values[:mid + 1]
        else:
            new_child.children = child.children[mid + 1:]
            child.children = child.children[:mid + 1]

        # persist both
        self.disk.write_page(child_offset, child)
        self.disk.write_page(new_offset, new_child)
