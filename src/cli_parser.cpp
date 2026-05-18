#include "cli_parser.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace Cli;

Parser::Parser() {
	_cs_buf.reserve(_cs_buffer_size);
	_ns_buf.reserve(_ns_buffer_size);
}

void Parser::setOnReadyCallback(OnReadyCallback f) {
	_on_ready_callback = f;
}

void Parser::setOnEventCallback(OnEventCallback f) {
	_on_event_callback = f;
}

void onReadySequenceCallback(const char *buf, uint8_t *state = nullptr) {
	if (state) {
		*state |= Parser::State::FINAL;
	}
	// printf("onReadySequenceCallback: %s\n", buf);
	std::cout << "onReadySequenceCallback: " << buf << std::endl;
}

int Parser::collect(const char c) {

	if (_state & State::FINAL) {
		_state = State::NONE;
		_cs_buf.clear();
	}

	_cs_buf.push_back(c);

	if (c == static_cast<char>(CtlCh::ESC)) {
		_state |= State::ESC;
		_cs_buf.clear(); // for printable output
		return _state & State::ESC;
	} else {
		if (!(_state & State::ESC)) {
			_cs_buf.clear();
		}
	}

	if (_state & State::ESC && !(_state & (State::CSI | State::OSC))) {

		switch (static_cast<CtlSq>(c)) {
		case CtlSq::CSI:
			_state |= State::CSI;
			return _state & State::ESC;
		case CtlSq::OSC:
			_state |= State::OSC;
			return _state & State::ESC;
		case CtlSq::IND:
		case CtlSq::NEL:
		case CtlSq::HTS:
		case CtlSq::RI:
		case CtlSq::SS2:
		case CtlSq::SS3:
		case CtlSq::DCS:
		case CtlSq::SPA:
		case CtlSq::EPA:
		case CtlSq::SOS:
		case CtlSq::DECID:
		case CtlSq::ST:
		case CtlSq::PM:
		case CtlSq::APC:

		//
		case CtlSq::DECBI:
		case CtlSq::DECSC:
		case CtlSq::DECRC:
		case CtlSq::DECFI:
		case CtlSq::DECKPAM:
		case CtlSq::DECKPNM:
		case CtlSq::RIS:
		case CtlSq::ML:
		case CtlSq::MU:
		case CtlSq::LS2:
		case CtlSq::LS3:
		case CtlSq::LS3R:
		case CtlSq::LS2R:
		case CtlSq::LS1R:
			// _state |= State::FINAL;
			onReadySequenceCallback(_cs_buf.data(), &_state);
			break;
			// default:
			// 	break;
		}
	}

	if (_state & State::CSI) {
		switch (static_cast<CSISqFinal>(c)) {
		case CSISqFinal::ICH:
		case CSISqFinal::CUU:
		case CSISqFinal::CUD:
		case CSISqFinal::CUF:
		case CSISqFinal::CUB:
		case CSISqFinal::CNL:
		case CSISqFinal::CPL:
		case CSISqFinal::CHA:
		case CSISqFinal::CUP:
		case CSISqFinal::ED:
		case CSISqFinal::EL:
		case CSISqFinal::IL:
		case CSISqFinal::DL:
		case CSISqFinal::DCH:
		case CSISqFinal::ECH:
		case CSISqFinal::HPR:
		case CSISqFinal::DA:
		case CSISqFinal::VPA:
		case CSISqFinal::VPR:
		case CSISqFinal::HVP:
		case CSISqFinal::TBC:
		case CSISqFinal::SM:
		case CSISqFinal::RM:
		case CSISqFinal::SGR:
		case CSISqFinal::DSR:
		case CSISqFinal::DECLL:
		case CSISqFinal::DECSTBM:
		case CSISqFinal::SCOSC:
		case CSISqFinal::SCORC:
		case CSISqFinal::HPA:
			// _state |= State::FINAL;
			onReadySequenceCallback(_cs_buf.data(), &_state);
			break;
		}
	}

	if (_state & State::OSC) {
		switch (static_cast<CSISqFinal>(c)) { // OSCSqFinal
		default:
			// _state |= State::FINAL;
			onReadySequenceCallback(_cs_buf.data(), &_state);
			break;
		}
	}

	return _state & State::ESC;
}

int Parser::parseData(const char *data, size_t size) {
	if (!_on_ready_callback) {
		return -1;
	}

	for (size_t i = 0; i < size; ++i) {
		const unsigned char __c = data[i];
		bool collecting = collect(__c);
		switch (static_cast<CtlCh>(__c)) {
		case CtlCh::LF:
			break;
		case CtlCh::VT:
			break;
		case CtlCh::FF:
			break;
		case CtlCh::CR:
			break;
		case CtlCh::BEL:
			break;
		case CtlCh::BS:
			break;
		case CtlCh::SO:
			break;
		case CtlCh::SI:
			break;
		default:
			if (collecting) {
				break;
			}
			// unicode stacking
			if (__c >= 0x80) {
				_ns_buf.push_back(__c);
			} else {
				if (!_ns_buf.empty()) {
					int res = _on_ready_callback(_ns_buf.data(), _ns_buf.size());
					_ns_buf.clear();
					return res;
				}
			}
			break;
		}
	}

	// return _on_ready_callback(data, size);
	return 0;
}

int Parser::processEventData(std::vector<char> &data, int key_event) {

	// if (DECCKM & m_seq_key->modes()) {
	// 	seq_key = m_ss3;
	// } else {
	// 	seq_key = m_csi;
	// }

	// const char *seq = m_csi;
	// const size_t seq_s = sizeof(m_csi);

	const size_t seq_ss3_s = sizeof(m_ss3);
	const size_t seq_csi_s = sizeof(m_csi);

	switch (static_cast<Key>(key_event)) {
	case Key::Up: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		data.push_back(static_cast<char>(CSISqFinal::CUU));
	} break;
	case Key::Down: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		data.push_back(static_cast<char>(CSISqFinal::CUD));
	} break;
	case Key::Right: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		data.push_back(static_cast<char>(CSISqFinal::CUF));
	} break;
	case Key::Left: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		data.push_back(static_cast<char>(CSISqFinal::CUB));
	} break;
	case Key::Home: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		data.push_back(static_cast<char>(CSISqFinal::CUP));
	} break;
	case Key::End: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		data.push_back(static_cast<char>(CSISqFinal::CPL));
	} break;
	//
	case Key::F1: {
		// data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		// const char vt[] = {'1', '1', '~'};
		// data.insert(data.end(), vt, &vt[sizeof(vt)]);

		data.insert(data.end(), m_ss3, &m_ss3[seq_ss3_s]);
		data.push_back('P');
	} break;
	case Key::F2: {
		// data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		// const char vt[] = {'1', '2', '~'};
		// data.insert(data.end(), vt, &vt[sizeof(vt)]);

		data.insert(data.end(), m_ss3, &m_ss3[seq_ss3_s]);
		data.push_back('Q');
	} break;
	case Key::F3: {
		// data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		// const char vt[] = {'1', '3', '~'};
		// data.insert(data.end(), vt, &vt[sizeof(vt)]);

		data.insert(data.end(), m_ss3, &m_ss3[seq_ss3_s]);
		data.push_back('R');
	} break;
	case Key::F4: {
		// data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		// const char vt[] = {'1', '4', '~'};
		// data.insert(data.end(), vt, &vt[sizeof(vt)]);

		data.insert(data.end(), m_ss3, &m_ss3[seq_ss3_s]);
		data.push_back('S');
	} break;
	//
	case Key::F5: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		const char vt[] = {'1', '5', '~'};
		data.insert(data.end(), vt, &vt[sizeof(vt)]);
	} break;
	case Key::F6: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		const char vt[] = {'1', '7', '~'};
		data.insert(data.end(), vt, &vt[sizeof(vt)]);
	} break;
	case Key::F7: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		const char vt[] = {'1', '8', '~'};
		data.insert(data.end(), vt, &vt[sizeof(vt)]);
	} break;
	case Key::F8: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		const char vt[] = {'1', '9', '~'};
		data.insert(data.end(), vt, &vt[sizeof(vt)]);
	} break;
	case Key::F9: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		const char vt[] = {'2', '0', '~'};
		data.insert(data.end(), vt, &vt[sizeof(vt)]);
	} break;
	case Key::F10: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		const char vt[] = {'2', '1', '~'};
		data.insert(data.end(), vt, &vt[sizeof(vt)]);
	} break;
	case Key::F11: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		const char vt[] = {'2', '3', '~'};
		data.insert(data.end(), vt, &vt[sizeof(vt)]);
	} break;
	case Key::F12: {
		data.insert(data.end(), m_csi, &m_csi[seq_csi_s]);
		const char vt[] = {'2', '4', '~'};
		data.insert(data.end(), vt, &vt[sizeof(vt)]);
	} break;
	}

	if (!_on_event_callback) {
		return -1;
	}

	return _on_event_callback(data.data(), data.size());
}
