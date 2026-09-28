import argparse
import os

# Set debug level has 0 run on in production
os.environ["FLASK_DEBUG"] = "1"

from temperature_and_humidity import app  # noqa E402

if "FLASK_RUN_HOST" in os.environ:
    app.run(host=os.environ["FLASK_RUN_HOST"], port=8000)
else:
    app.run(port=8000)
