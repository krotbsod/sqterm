#include "cli_parser.hpp"
#include <cstdint>
#include <cstdlib>

using namespace Cli;

void Parser::setOnReadyCallback(OnReadyCallback f) {
	_on_ready_callback = f;
}

void Parser::setOnEventCallback(OnEventCallback f) {
	_on_event_callback = f;
}

int Parser::parseData(const char *data, size_t size) {
	if (!_on_ready_callback) {
		return -1;
	}

	// do something. now only proxy data

	return _on_ready_callback(data, size);
}

int Parser::processEventData(std::vector<char> &data, int key_event) {

	// if (DECCKM & m_seq_key->modes()) {
	// 	seq_key = m_ss3;
	// } else {
	// 	seq_key = m_csi;
	// }

	const char *seq = m_csi;
	const size_t seq_s = sizeof(m_csi);

	const size_t seq_ss3_s = sizeof(m_ss3);
	const size_t seq_csi_s = sizeof(m_csi);

	switch (static_cast<Key>(key_event)) {
	case Key::Up: {
		data.insert(data.end(), seq, &seq[seq_s]);
		data.push_back(static_cast<char>(CsiKey::Up));
	} break;
	case Key::Down: {
		data.insert(data.end(), seq, &seq[seq_s]);
		data.push_back(static_cast<char>(CsiKey::Down));
	} break;
	case Key::Right: {
		data.insert(data.end(), seq, &seq[seq_s]);
		data.push_back(static_cast<char>(CsiKey::Right));
	} break;
	case Key::Left: {
		data.insert(data.end(), seq, &seq[seq_s]);
		data.push_back(static_cast<char>(CsiKey::Left));
	} break;
	case Key::Home: {
		data.insert(data.end(), seq, &seq[seq_s]);
		data.push_back(static_cast<char>(CsiKey::Home));
	} break;
	case Key::End: {
		data.insert(data.end(), seq, &seq[seq_s]);
		data.push_back(static_cast<char>(CsiKey::End));
	} break;
	//
	case Key::F1: {
		data.insert(data.end(), m_ss3, &m_ss3[seq_ss3_s]);
		data.push_back('P');
	} break;
	case Key::F2: {
		data.insert(data.end(), m_ss3, &m_ss3[seq_ss3_s]);
		data.push_back('Q');
	} break;
	case Key::F3: {
		data.insert(data.end(), m_ss3, &m_ss3[seq_ss3_s]);
		data.push_back('R');
	} break;
	case Key::F4: {
		data.insert(data.end(), m_ss3, &m_ss3[seq_ss3_s]);
		data.push_back('S');
	} break;
	//
	case Key::F5: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		char k[] = "15~";
		data.insert(data.end(), k, &k[sizeof(k)]);
	} break;
	case Key::F6: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		char k[] = "17~";
		data.insert(data.end(), k, &k[sizeof(k)]);
	} break;
	case Key::F7: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		char k[] = "18~";
		data.insert(data.end(), k, &k[sizeof(k)]);
	} break;
	case Key::F8: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		char k[] = "19~";
		data.insert(data.end(), k, &k[sizeof(k)]);
	} break;
	case Key::F9: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		char k[] = "20~";
		data.insert(data.end(), k, &k[sizeof(k)]);
	} break;
	case Key::F10: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		char k[] = "21~";
		data.insert(data.end(), k, &k[sizeof(k)]);
	} break;
	case Key::F11: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		char k[] = "23~";
		data.insert(data.end(), k, &k[sizeof(k)]);
	} break;
	case Key::F12: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		char k[] = "24~";
		data.insert(data.end(), k, &k[sizeof(k)]);
	} break;
	}
	// data.push_back('\0');

	return 0;
}
