

#include <./morseCode/morseCode.h>
#include <../server/router/router.h>

namespace morseCode
{
    class Router : public EspWeb::Router
    {
    public:
        Router()
        {
            route("POST", "/morse", post_Morse);
            route("GET", "/morse", get_Morse);
        }

    private:
        static void post_Morse(EspWeb::Request &req, EspWeb::Response &res)
        {
            const gpio_num_t GPIO = GPIO_NUM_2;
            const std::string message = req.body.json()["message"];

            Serial.printf("Attempting to morse on LED on GPIO %d\n", GPIO);
            Serial.printf("Message: %s\n", message.c_str());

            Morse mc(GPIO);
            mc.morse(message);

            Serial.printf("LED should be off now.\n");

            res.status(200);
        }

        static void get_Morse(EspWeb::Request &req, EspWeb::Response &res)
        {
            res.html(R"html(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Morse Code</title>
  <style>
    body { font-family: monospace; max-width: 480px; margin: 60px auto; padding: 0 16px; background: #111; color: #eee; }
    h1 { font-size: 1.4rem; margin-bottom: 24px; letter-spacing: 2px; }
    input { width: 100%; box-sizing: border-box; padding: 10px; font-size: 1rem; font-family: monospace; background: #222; color: #eee; border: 1px solid #444; border-radius: 4px; }
    button { margin-top: 12px; width: 100%; padding: 10px; font-size: 1rem; font-family: monospace; background: #2a6; color: #fff; border: none; border-radius: 4px; cursor: pointer; }
    button:disabled { background: #444; cursor: not-allowed; }
    #status { margin-top: 16px; font-size: 0.9rem; color: #aaa; min-height: 1.2em; }
  </style>
</head>
<body>
  <h1>-- --- .-. ... .</h1>
  <input id="msg" type="text" placeholder="Text eingeben..." autofocus />
  <button id="btn" onclick="send()">Senden</button>
  <div id="status"></div>
  <script>
    async function send() {
      const msg = document.getElementById('msg').value.trim();
      if (!msg) return;
      const btn = document.getElementById('btn');
      const status = document.getElementById('status');
      btn.disabled = true;
      status.textContent = 'Wird gemorst...';
      try {
        const res = await fetch('/morse', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ message: msg })
        });
        status.textContent = res.ok ? 'Fertig.' : 'Fehler: ' + res.status;
      } catch (e) {
        status.textContent = 'Verbindungsfehler.';
      }
      btn.disabled = false;
    }
    document.getElementById('msg').addEventListener('keydown', e => { if (e.key === 'Enter') send(); });
  </script>
</body>
</html>
)html").status(200);
        }
    };
}
