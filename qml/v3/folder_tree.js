.pragma library

// Flattens folder rows into a pre-ordered, depth-tagged tree gated by an
// expanded set. `folders` is an array of plain objects that each carry at least
// folderId, parentId, position, and folderType; every field is copied through
// to the output row. `expanded` maps folderId -> false to collapse a subtree.
// `manualOnly` drops smart folders (folderType 1) and reparents any survivor
// whose parent was dropped up to the root, so nothing is orphaned.
//
// Each output row is the input object plus depth, hasChildren, and expanded.
function flatten(folders, expanded, manualOnly) {
    var included = {};
    var items = [];
    folders.forEach(function (f) {
        if (manualOnly && f.folderType !== 0) {
            return;
        }
        included[f.folderId] = true;
        items.push(f);
    });

    var byParent = {};
    items.forEach(function (f) {
        var p = (f.parentId === -1 || included[f.parentId]) ? f.parentId : -1;
        (byParent[p] = byParent[p] || []).push(f);
    });
    Object.keys(byParent).forEach(function (k) {
        byParent[k].sort(function (a, b) {
            return a.position - b.position;
        });
    });

    var out = [];
    function walk(pid, depth) {
        (byParent[pid] || []).forEach(function (f) {
            var kids = byParent[f.folderId] || [];
            var isExpanded = expanded[f.folderId] !== false;
            var row = {};
            for (var key in f) {
                row[key] = f[key];
            }
            row.depth = depth;
            row.hasChildren = kids.length > 0;
            row.expanded = isExpanded;
            out.push(row);
            if (isExpanded) {
                walk(f.folderId, depth + 1);
            }
        });
    }
    walk(-1, 0);
    return out;
}
