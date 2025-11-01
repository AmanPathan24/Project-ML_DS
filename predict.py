import sys, json, joblib, pandas as pd

def main():
    if len(sys.argv) < 2:
        print(json.dumps({"error": "Usage: python predict.py features.json"}))
        sys.exit(1)

    features_file = sys.argv[1]
    with open(features_file, 'r') as f:
        features = json.load(f)

    df = pd.DataFrame([features])
    pipeline = joblib.load('pipeline.pkl')

    try:
        pred = pipeline.predict(df)
        price = float(pred[0])
        print(json.dumps({"predicted_price": price}))
    except Exception as e:
        print(json.dumps({"error": str(e)}))
        sys.exit(1)

if __name__ == "__main__":
    main()