import json
import os
import sys
import numpy as np
from sklearn.ensemble import RandomForestClassifier, RandomForestRegressor
import joblib

# 1. Wo liegt dieses Skript gerade? (z.B. .../src_python/ml_python/)
current_dir = os.path.dirname(os.path.abspath(__file__))

# 2. Dynamische Pfadberechnung zum globalen 'data'-Ordner auf Root-Ebene
# Wenn C++ das Skript aus der Build-Kopie startet, müssen wir 3 Ebenen hoch.
# Wenn du es manuell aus src_python startest, müssen wir 2 Ebenen hoch.
if "build" in current_dir:
    project_root = os.path.abspath(os.path.join(current_dir, "..", "..", ".."))
else:
    project_root = os.path.abspath(os.path.join(current_dir, "..", ".."))

# Sys-Path für die Imports (benötigt das Hauptverzeichnis von src_python)
src_python_dir = os.path.join(project_root, "src_python")
if src_python_dir not in sys.path:
    sys.path.append(src_python_dir)

from ml_python.feature_builder import build_features

if len(sys.argv) < 2:
    raise ValueError("teamCount argument missing")

team_count = int(sys.argv[1])

# ==============================================================================
# HIER SIND DIE NEUEN KORREKTEN PFADE:
# ==============================================================================
# Die Datensätze liegen im globalen data-Ordner auf Root-Ebene
data_path = os.path.join(project_root, "data", f"team_{team_count}.jsonl")

# Das Modell bleibt als Code-Bestandteil lokal im selben Ordner wie dieses Skript
model_path = os.path.join(current_dir, f"ml_model_{team_count}.joblib")
# ==============================================================================

raw_samples = []
# Sicherheits-Check: Falls die Datei noch gar nicht existiert, fangen wir das sauber ab
if not os.path.exists(data_path):
    raise FileNotFoundError(f"[TRAIN] FEHLER: Didn't find samples under: {data_path}")

with open(data_path, "r") as f:
    for line in f:
        if line.strip():
            raw_samples.append(json.loads(line))

print(f"[TRAIN] Loaded {len(raw_samples)} raw samples from {data_path}")

sample_groups = {}

for sample in raw_samples:
    features = build_features(sample)
    situation_key = tuple(np.round(features, decimals=0))
    
    if situation_key not in sample_groups:
        sample_groups[situation_key] = []
    sample_groups[situation_key].append(sample)

filtered_samples = []
for situation_key, group in sample_groups.items():
    best_sample = max(group, key=lambda s: s.get("evaluationScore", -999.0))
    if best_sample.get("evaluationScore", -999.0) > -50.0:
        filtered_samples.append(best_sample)

print(f"[TRAIN] Filtered from {len(raw_samples)} to {len(filtered_samples)} best-action samples")

label_map = {
    "MoveForward": 0, "RotateLeft": 1, "RotateRight": 2, 
    "Shoot": 3, "StandStill": 4, "None": 5
}

X = []
y_type = []
y_value = []

for sample in filtered_samples:
    features = build_features(sample)
    X.append(features)
    y_type.append(label_map.get(sample["actionType"], 5))
    y_value.append(sample["actionValue"])

X = np.array(X, dtype=np.float32)
y_type = np.array(y_type)
y_value = np.array(y_value, dtype=np.float32)

print(f"[TRAIN] Training models with shape {X.shape}...")
clf_type = RandomForestClassifier(n_estimators=100, n_jobs=1)
clf_value = RandomForestRegressor(n_estimators=100, n_jobs=1)

clf_type.fit(X, y_type)
clf_value.fit(X, y_value)

joblib.dump((clf_type, clf_value), model_path)
print(f"[TRAIN] Success! Model saved: {model_path}")