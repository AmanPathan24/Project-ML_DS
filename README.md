# House Price Estimator (DSA + ML demo)

Small educational project that demonstrates integrating a C++ front-end with a Python ML predictor and compares basic data structure search strategies on a CSV dataset.
House Price Estimator: C++ frontend writes feature JSON, calls a Python sklearn pipeline (pipeline.pkl) for price prediction, then searches a CSV dataset using vector, linked list, BST, and hash-table to find listings within ±10% of predicted price and reports matches and performance metrics.

## Repository layout
- `main.cpp` — C++ driver: writes input JSON, calls `predict.py`, loads `dataset.csv`, and compares search using:
  - Vector (linear scan)
  - Singly linked list
  - Binary Search Tree (BST)
  - Hash table (bucketed by coarse price range)
- `predict.py` — Python script that loads `pipeline.pkl` and returns JSON with `predicted_price`
- `dataset.csv` — CSV of listings used for nearest matches
- `temp_features.json` — intermediary JSON written by `main.cpp`
- `pipeline.pkl` — trained sklearn pipeline (must be provided)

## Requirements
- C++ compiler (g++, clang, or MSVC) supporting C++11+
- Python 3.11 (64-bit recommended) with:
  - pandas, joblib and scikit-learn (pipeline dependencies)
- On Windows: ensure the Python on PATH is the one you intend to use (name may be `python` or `python3`).

Note: On Windows, 32-bit Python will force building some packages from source (e.g., recent NumPy) and can fail. Use 64-bit Python or conda to avoid build issues.

## Build (Windows PowerShell example)
1. Create/activate Python venv (recommended)
```powershell
python -m venv .venv
.\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip setuptools wheel
python -c "import struct,platform,sys; print(struct.calcsize('P')*8, platform.python_implementation(), sys.version)"
```

2. Install Python deps (example)
```powershell
pip install pandas joblib scikit-learn
# If you hit NumPy build errors, use a 64-bit Python or conda environment.
```

3. Compile C++:
```powershell
g++ -std=c++11 main.cpp -O2 -o estimator.exe
```

## Run
- Ensure `pipeline.pkl` and `dataset.csv` are present in the project folder.
- Execute:
```powershell
.\estimator.exe
```
The C++ program writes `temp_features.json`, calls `python predict.py temp_features.json`, parses output, searches `dataset.csv` for listings within ±10% of predicted price, prints top matches and a performance comparison.

## predict.py contract
- Accepts one argument: path to JSON features file.
- Loads JSON → DataFrame, then `joblib.load("pipeline.pkl")` and `pipeline.predict(df)`.
- Prints JSON containing `{"predicted_price": <float>}` on success.

## Data/JSON format
- `temp_features.json` is a flat mapping of feature names → values (numbers unquoted where possible).
- `dataset.csv` expected columns: City,Price,Locality (first row header).

## Notes & troubleshooting
- If `dataset.csv` missing, the program exits with an error.
- If Python is not on PATH or named differently, update the command in `main.cpp` (function `runPythonAndGetOutput`).
- For large datasets or accurate price bucketing, adjust bucket size (`/100000`) and tolerance in `main.cpp`.
- Consider using a JSON library (nlohmann/json) in C++ to improve robustness.
- To avoid NumPy wheel build problems on Windows, install 64-bit Python or use conda:
```powershell
conda create -n est python=3.11
conda activate est
conda install -c conda-forge pandas numpy scikit-learn joblib
```

## Extending the project
- Replace hard-coded features in `main.cpp` with interactive input or file-based input presets (`sample_features.json`).
- Persist and version training pipeline (`pipeline.pkl`) and include model training script.
- Replace unbalanced BST with balanced tree (AVL/Red-Black) or use sorted containers for faster range queries.
