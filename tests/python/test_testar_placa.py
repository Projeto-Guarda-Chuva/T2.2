import contextlib
import importlib.util
import io
import sys
import types
import unittest
from pathlib import Path
from unittest.mock import patch


MODULE_PATH = (
    Path(__file__).resolve().parents[2] / "Iluminacao" / "Tela" / "Teste" / "testar_placa.py"
)

sys.dont_write_bytecode = True


def load_module(fake_requests: types.ModuleType):
    spec = importlib.util.spec_from_file_location("testar_placa_under_test", MODULE_PATH)
    module = importlib.util.module_from_spec(spec)
    sys.modules["requests"] = fake_requests
    spec.loader.exec_module(module)
    return module


class RequestsStub(types.ModuleType):
    class Exceptions:
        class ConnectionError(Exception):
            pass

        class Timeout(Exception):
            pass

    def __init__(self):
        super().__init__("requests")
        self.exceptions = self.Exceptions
        self.last_call = None
        self.next_result = None

    def post(self, url, json, timeout):
        self.last_call = {"url": url, "json": json, "timeout": timeout}
        if isinstance(self.next_result, Exception):
            raise self.next_result
        return self.next_result


class ResponseStub:
    def __init__(self, status_code=200, text="OK"):
        self.status_code = status_code
        self.text = text


class TestarPlacaTests(unittest.TestCase):
    def setUp(self):
        self.requests_stub = RequestsStub()
        self.module = load_module(self.requests_stub)

    def tearDown(self):
        sys.modules.pop("requests", None)
        sys.modules.pop("testar_placa_under_test", None)

    def test_enviar_makes_post_request_with_payload(self):
        self.requests_stub.next_result = ResponseStub(200, "OK")
        output = io.StringIO()

        with contextlib.redirect_stdout(output):
            self.module.enviar({"id": 13})

        self.assertEqual(
            self.requests_stub.last_call,
            {
                "url": "http://192.168.76.43/cmd",
                "json": {"id": 13},
                "timeout": 5,
            },
        )
        self.assertIn("Resposta: 200 OK", output.getvalue())

    def test_enviar_handles_connection_error(self):
        self.requests_stub.next_result = self.requests_stub.exceptions.ConnectionError()
        output = io.StringIO()

        with contextlib.redirect_stdout(output):
            self.module.enviar({"id": 11})

        self.assertIn("nao foi possivel conectar", output.getvalue())

    def test_enviar_handles_timeout(self):
        self.requests_stub.next_result = self.requests_stub.exceptions.Timeout()
        output = io.StringIO()

        with contextlib.redirect_stdout(output):
            self.module.enviar({"id": 12})

        self.assertIn("Erro: timeout", output.getvalue())

    def test_main_sends_simple_command(self):
        sent_payloads = []

        def fake_enviar(payload):
            sent_payloads.append(payload)

        with patch.object(self.module, "enviar", side_effect=fake_enviar):
            with patch("builtins.input", side_effect=["13", "0"]):
                output = io.StringIO()
                with contextlib.redirect_stdout(output):
                    self.module.main()

        self.assertEqual(sent_payloads, [{"id": 13}])
        self.assertIn("Saindo.", output.getvalue())

    def test_main_clamps_custom_rgb_values(self):
        sent_payloads = []

        def fake_enviar(payload):
            sent_payloads.append(payload)

        with patch.object(self.module, "enviar", side_effect=fake_enviar):
            with patch("builtins.input", side_effect=["10", "300", "-5", "128", "0"]):
                with contextlib.redirect_stdout(io.StringIO()):
                    self.module.main()

        self.assertEqual(
            sent_payloads,
            [{"id": 10, "r": 255, "g": 0, "b": 128}],
        )

    def test_main_handles_invalid_menu_input(self):
        with patch.object(self.module, "enviar") as fake_enviar:
            with patch("builtins.input", side_effect=["abc", "0"]):
                output = io.StringIO()
                with contextlib.redirect_stdout(output):
                    self.module.main()

        fake_enviar.assert_not_called()
        self.assertIn("Valor invalido.", output.getvalue())
