#include "transport.h"

#define _weak __attribute__((weak))

_weak ctx_t *websocket_init(void) {};
_weak int	websocket_deinit(ctx_t *ctx) {};
_weak ses_t *websocket_open(ctx_t *ctx, const char *url, const char *ext_h, cb_set_t *cb_set, void *usr_data) {};
_weak int	websocket_close(ses_t *session) {};
_weak int	websocket_write_text(ses_t *session, const char *message) {};
_weak int	websocket_write_bin(ses_t *session, const uint8_t *data, size_t size) {};
_weak int	websocket_read(ses_t *session, uint8_t *recv_buffer, size_t read_bytes, uint8_t *type) {};

_weak ctx_t *mqtt_init(void) {};
_weak int	mqtt_deinit(ctx_t *ctx) {};
_weak ses_t *mqtt_open(ctx_t *ctx, const char *url, const char *ext_h, cb_set_t *cb_set, void *usr_data) {};
_weak int	mqtt_close(ses_t *session) {};
_weak int	mqtt_write_text(ses_t *session, const char *message) {};
_weak int	mqtt_write_bin(ses_t *session, const uint8_t *data, size_t size) {};
_weak int	mqtt_read(ses_t *session, uint8_t *recv_buffer, size_t read_bytes, uint8_t *type) {};

static trans_inf_t websocket_opts = {
	.init		=	websocket_init,
	.deinit		=	websocket_deinit,
	.open		=	websocket_open,
	.close		=	websocket_close,
	.read		=	websocket_read,
	.write_bin	=	websocket_write_bin,
	.write_text =	websocket_write_text,
};

static trans_inf_t mqtt_opts = {
	.init		=	mqtt_init,
	.deinit		=	mqtt_deinit,
	.open		=	mqtt_open,
	.close		=	mqtt_close,
	.read		=	mqtt_read,
	.write_bin	=	mqtt_write_bin,
	.write_text =	mqtt_write_text,
};

trans_context_t *trans_init(network_protocol_type type)
{
	trans_context_t *trans_ctx = 
		(trans_context_t *)trans_malloc(sizeof(trans_context_t));

	if (trans_ctx == NULL) {
		TRANS_LOGE("malloc(%d) failed.\n", sizeof(trans_context_t));
		return NULL;
	}

	trans_memset(trans_ctx, 0, sizeof(trans_context_t));

	switch (type) {
		case WEBSOCKET: trans_ctx->inf = &websocket_opts;	break;
		case MQTT:		trans_ctx->inf = &mqtt_opts;		break;
		default:		trans_ctx->inf = &websocket_opts;	break;
	}

	if (trans_ctx->inf->init) {
		ctx_t *ctx = trans_ctx->inf->init();

		if (ctx)
			trans_ctx->inf_ctx = ctx;
		else
			goto exit0;
	} else
		goto exit0;

	return trans_ctx;
exit0:
	trans_free(trans_ctx);
	return NULL;
}

int trans_deinit(trans_context_t *trans_ctx)
{
	if (trans_ctx == NULL)
		return TRANS_ERROR;

	if (trans_ctx->inf->deinit) {
		trans_ctx->inf->deinit(trans_ctx->inf_ctx);
		trans_ctx->inf_ctx = NULL;
	}

	trans_free(trans_ctx);

	return TRANS_SUCCESS;
}

trans_session_t *trans_open(trans_context_t *trans_ctx, const char *url, const char *ext_h,
		cb_set_t *cb_set, void *usr_data)
{
	if (trans_ctx == NULL || url == NULL)
		return NULL;

	trans_session_t *session = 
		(trans_session_t *)trans_malloc(sizeof(trans_session_t));

	if (session == NULL) {
		TRANS_LOGE("malloc(%d) failed.\n", sizeof(trans_session_t));
		return NULL;
	}

	trans_memset(session, 0, sizeof(trans_session_t));

	session->trans_ctx = trans_ctx;

	if (trans_ctx->inf->open) {
		session->inf_sess = trans_ctx->inf->open(trans_ctx->inf_ctx, url, ext_h, cb_set, usr_data);

		if (session->inf_sess == NULL) {
			trans_free(session);
			TRANS_LOGE("open failed.\n");
			return NULL;
		}
	}

	return session;
}

int trans_close(trans_session_t *trans_sess)
{
	if (trans_sess == NULL)
		return TRANS_ERROR;

	if (trans_sess->trans_ctx->inf->close) {
		trans_sess->trans_ctx->inf->close(trans_sess->inf_sess);
		trans_sess->inf_sess = NULL;
	}

	trans_free(trans_sess);

	return TRANS_SUCCESS;
}

int trans_write_text(trans_session_t *trans_sess, const char *message)
{
	int bytes_sent = 0;

	if (trans_sess == NULL)
		return TRANS_ERROR;

	if (trans_sess->trans_ctx->inf->write_text) {
		bytes_sent = trans_sess->trans_ctx->inf->write_text(trans_sess->inf_sess, message);
	}

	return bytes_sent;
}

int trans_write_bin(trans_session_t *trans_sess, const uint8_t *data, size_t size)
{
	int bytes_sent = 0;

	if (trans_sess == NULL)
		return TRANS_ERROR;

	if (trans_sess->trans_ctx->inf->write_bin) {
		bytes_sent = trans_sess->trans_ctx->inf->write_bin(trans_sess->inf_sess, data, size);
	}

	return bytes_sent;
}

int trans_read(trans_session_t *trans_sess, uint8_t *recv_buffer, size_t read_bytes, uint8_t *type)
{
	int bytes_received = 0;

	if (trans_sess == NULL)
		return TRANS_ERROR;

	if (trans_sess->trans_ctx->inf->read) {
		bytes_received = trans_sess->trans_ctx->inf->read(trans_sess->inf_sess, recv_buffer, read_bytes, type);
	}

	return bytes_received;
}

int trans_set_userobj(trans_session_t *trans_sess, void *user_obj)
{
	if(trans_sess) {
		trans_sess->user_obj = user_obj;
	} else {
		return TRANS_ERROR;
	}

	return TRANS_SUCCESS;
}

void *trans_get_userobj(trans_session_t *trans_sess)
{
	void *user_obj = NULL;

	if(trans_sess) {
		user_obj = trans_sess->user_obj;
	} else {
		return NULL;
	}

	return user_obj;
}
