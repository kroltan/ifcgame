import json
import glob



def update_registry(registry, new):
	for modname, module in new.items():
		if not modname in registry:
			registry[modname] = module.copy()

		local_module = registry[modname]
		for item in local_module:
			if any(t is not item and item["name"] == t["name"] for t in local_module):
				local_module.remove(item)

if __name__ == "__main__":
	modules = {}
	for path in glob.iglob("./include/scripting/*.json"):
		with open(path, "r") as f:
			update_registry(modules, json.load(f))
